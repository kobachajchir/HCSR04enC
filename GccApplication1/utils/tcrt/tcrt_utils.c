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

// 	Byte_Flag_Struct statusFlags; //Aca tenemos el estado en el nibble H, y las banderas en el nibble L
// 	uint16_t ADCConvertedValue; //Resolucion 10 bits 0 - 1023
// 	uint8_t channel; //Canal TCRT_X_CHANNEL
// 	uint8_t pin;
// }TCRT_t;

void TCRT_init_Handlers(){
	IR_A.flags.byte = 0;
	IR_A.channel = TCRT_A_CHANNEL;
	IR_A.pin = TCRT_A;
	IR_A.ADCConvertedValue = 0;
	NIBBLEH_SET_STATE(IR_A.flags, TCRT_STATUS_IDLE);
	SET_FLAG(IR_A.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED)){
		printf("TCRT A ENABLED");
	}
	IR_B.flags.byte = 0;
	IR_B.channel = TCRT_B_CHANNEL;
	IR_B.pin = TCRT_B;
	IR_B.ADCConvertedValue = 0;
	NIBBLEH_SET_STATE(IR_B.flags, TCRT_STATUS_IDLE);
	SET_FLAG(IR_B.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED)){
		printf("TCRT B ENABLED");
	}
	IR_C.flags.byte = 0;
	IR_C.channel = TCRT_C_CHANNEL;
	IR_C.pin = TCRT_C;
	IR_C.ADCConvertedValue = 0;
	NIBBLEH_SET_STATE(IR_C.flags, TCRT_STATUS_IDLE);
	SET_FLAG(IR_C.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED)){
		printf("TCRT C ENABLED");
	}
	IR_U.flags.byte = 0;
	IR_U.channel = TCRT_U_CHANNEL;
	IR_U.pin = TCRT_U;
	IR_U.ADCConvertedValue = 0;
	NIBBLEH_SET_STATE(IR_U.flags, TCRT_STATUS_IDLE);
	SET_FLAG(IR_U.flags, TCRT_ENABLED);
	if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED)){
		printf("TCRT U ENABLED");
	}
	printf("Init TCRT Handlers");
}
