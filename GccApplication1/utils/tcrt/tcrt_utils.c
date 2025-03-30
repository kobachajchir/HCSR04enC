/*
 * tcrt_utils.c
 *
 * Created: 27-Mar-25 11:36:03 AM
 *  Author: kobac
 */ 

#include <stdio.h>
#include <avr/io.h>
#include "../../main.h"

void tcrt_init(void)
{
	// 1. Configuramos los pines PC0-PC3 como entrada
	DDRC &= ~((1 << TCRT_A | (1 << TCRT_B) | (1 << TCRT_C) | (1 << TCRT_U)));

	// 2. (Opcional) Desactivamos pull-up internos
	PORTC &= ~((1 << TCRT_A) | (1 << TCRT_B) | (1 << TCRT_C) | (1 << TCRT_U));

	// 3. Inicializamos el ADC (si no está hecho ya)
	ADMUX = (1 << REFS0);               // Referencia AVCC (5V)
	ADCSRA = (1 << ADEN) |              // Habilitar ADC
	(1 << ADPS2) | (1 << ADPS1); // Prescaler 64 (para 16 MHz ? 250 kHz ADC clk)
}

uint16_t tcrt_read_channel(uint8_t channel)
{
	// Asegurar que channel esté entre 0 y 3
	if (channel > 3) return 0;

	ADMUX = (ADMUX & 0xF0) | channel;   // Seleccionamos canal ADC0–ADC3

	ADCSRA |= (1 << ADSC);              // Iniciar conversión
	while (ADCSRA & (1 << ADSC));       // Esperar a que termine

	return ADC;                         // Retorna valor de 10 bits (0–1023)
}

void TCRT_init_Handlers(){
	IR_A.flags.byte = 0;
	IR_A.channel = TCRT_A_CHANNEL;
	IR_A.pin = TCRT_A;
	IR_A.ADCConvertedValue = 0;
	IR_A.lastReading = 0;
	IR_A.hysteresis_percent = 20;
	NIBBLEH_SET_STATE(IR_A.flags, TCRT_STATUS_IDLE);
	SET_FLAG(IR_A.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED)){
		printf("TCRT A ENABLED\n");
	}
	IR_B.flags.byte = 0;
	IR_B.channel = TCRT_B_CHANNEL;
	IR_B.pin = TCRT_B;
	IR_B.ADCConvertedValue = 0;
	IR_B.lastReading = 0;
	IR_B.hysteresis_percent = 20;
	
	NIBBLEH_SET_STATE(IR_B.flags, TCRT_STATUS_IDLE);
	SET_FLAG(IR_B.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED)){
		printf("TCRT B ENABLED\n");
	}
	IR_C.flags.byte = 0;
	IR_C.channel = TCRT_C_CHANNEL;
	IR_C.pin = TCRT_C;
	IR_C.ADCConvertedValue = 0;
	IR_C.lastReading = 0;
	IR_C.hysteresis_percent = 20;
	NIBBLEH_SET_STATE(IR_C.flags, TCRT_STATUS_IDLE);
	//SET_FLAG(IR_C.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED)){
		printf("TCRT C ENABLED\n");
	}
	IR_U.flags.byte = 0;
	IR_U.channel = TCRT_U_CHANNEL;
	IR_U.pin = TCRT_U;
	IR_U.ADCConvertedValue = 0;
	IR_U.lastReading = 0;
	IR_U.hysteresis_percent = 20;
	NIBBLEH_SET_STATE(IR_U.flags, TCRT_STATUS_IDLE);
	//SET_FLAG(IR_U.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED)){
		printf("TCRT U ENABLED\n");
	}
}

bool detect_rising_edge(TCRT_t* sensor, uint8_t current_state)
{
	bool rising = (sensor->lastReading == 0 && current_state == 1);
	sensor->lastReading = current_state;
	return rising;
}

void calibrateIRSensor(TCRT_t* sensor)
{
	if (IS_FLAG_SET(sensor->flags, TCRT_NEW_VALUE)) {
		CLEAR_FLAG(sensor->flags, TCRT_NEW_VALUE); // Se leyó, limpiar flag

		// Inicialización automática en primer ciclo
		if (sensor->calibrationCounter == 0) {
			sensor->lastReading = 0;
		}

		// Acumula valores
		sensor->lastReading += tcrt_read_channel(sensor->channel);
		sensor->calibrationCounter++;

		// Si terminó de tomar las muestras necesarias
		if (sensor->calibrationCounter >= TCRT_CALIBRATION_SAMPLES) {
			sensor->threshold = sensor->lastReading / TCRT_CALIBRATION_SAMPLES;
			sensor->calibrationCounter = 0;
			CLEAR_FLAG(sensor->flags, TCRT_CALIBRATING); // IMPORTANTE
			printf("Sensor canal %u calibrado con threshold %u\n", sensor->channel, sensor->threshold);
		}
	}
}

void tcrt_read(TCRT_t* sensor)
{
	uint16_t lectura = tcrt_read_channel(sensor->channel);
	sensor->lastReading = lectura;

	// Acumula
	sensor->filterAccumulator += lectura;
	sensor->calibrationCounter++;
		
	// Si alcanzó el número de muestras requerido
	if (sensor->calibrationCounter >= TCRT_FILTER_SAMPLES) {
		sensor->ADCConvertedValue = sensor->filterAccumulator / TCRT_FILTER_SAMPLES;
		sensor->filterAccumulator = 0;
		sensor->calibrationCounter = 0;
	}
}

bool tcrt_is_box_detected(TCRT_t* sensor)
{
	// Calcular el umbral ajustado con margen de histéresis
	uint32_t threshold_with_margin = (sensor->threshold * (100UL - sensor->hysteresis_percent)) / 100UL;

	// Ver si hay objeto ahora
	bool box_now = (sensor->ADCConvertedValue < threshold_with_margin);

	if (box_now && NIBBLEH_GET_STATE(sensor->flags) == TCRT_STATUS_IDLE) {
		// Caja recién detectada ? flanco de subida
		SET_FLAG(sensor->flags, TCRT_NEW_VALUE);
		SET_FLAG(sensor->flags, TCRT_EDGE_RISING);
		NIBBLEH_SET_STATE(sensor->flags, TCRT_READ);
	}
	else if (!box_now && NIBBLEH_GET_STATE(sensor->flags) == TCRT_READ) {
		// Caja recién salió del sensor ? flanco de bajada
		SET_FLAG(sensor->flags, TCRT_NEW_VALUE);
		CLEAR_FLAG(sensor->flags, TCRT_EDGE_RISING);
		NIBBLEH_SET_STATE(sensor->flags, TCRT_COUNTED);
	}

	return box_now;
}

