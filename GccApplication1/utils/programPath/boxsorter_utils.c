/*
 * boxSorter.c
 *
 * Created: 26-Mar-25 9:34:08 AM
 *  Author: kobac
 */ 

#include "../../main.h"
#include "../../types/boxTypes.h"
#include "../../types/sorterSystemTypes.h"
#include "../../types/ultrasonicDetectorType.h"
#include "boxsorter_utils.h"
#include <stdio.h>
#include <stdbool.h>

void initDetector(Ultrasonic_Detector_t* hcsr04Detector, ultrasonic_t* sensor_ultra, TCRT_t* sensor_IR){
	hcsr04Detector->sensor = &ultraSensor;
	hcsr04Detector->sensor_IR = &sensor_IR;
	hcsr04Detector->flags.byte = 0;
	NIBBLEH_SET_STATE(hcsr04Detector->flags, ULTRADET_SENSOR_IDLE);
	printf("Init hcsr04Detector, sensor idle\n");
}

void initOutputs(){
	initServo(&servoA, 0, SERVOA_PIN);
	initServo(&servoB, 1, SERVOB_PIN);
	initServo(&servoC, 2, SERVOC_PIN);
	salidaA.actuator_pin = SERVOA_PIN;
	salidaA.sensor_pin = IR_A.pin;
	salidaA.flags.byte = 0;
	salidaB.actuator_pin = SERVOB_PIN;
	salidaB.sensor_pin = IR_B.pin;
	salidaB.flags.byte = 0;
	salidaC.actuator_pin = SERVOC_PIN;
	salidaC.sensor_pin = IR_C.pin;
	salidaC.flags.byte = 0;
	printf("Init outputs\n");
}

void initSorter(sorter_system_t* SystemSorter){
	static box_height_range_matrix_t local_box_ranges = {
		.box_size_a = { BOX_A_MIN_MM, BOX_A_MAX_MM },
		.box_size_b = { BOX_B_MIN_MM, BOX_B_MAX_MM },
		.box_size_c = { BOX_C_MIN_MM, BOX_C_MAX_MM }
	};
	SystemSorter->box_ranges = &local_box_ranges;
	SystemSorter->outputs[0] = &salidaA;
	SystemSorter->outputs[1] = &salidaB;
	SystemSorter->outputs[2] = &salidaC;
	if(DEBUG_FLAGS_SORTER){
		printf("SYS SORTER DEBUG - Matriz de dimensiones iniciada\n");
		printf("SYS SORTER DEBUG - Salidas agregadas\n");
	}
}


box_type_t classify_box(uint8_t distance_mm, sorter_system_t* SystemSorter)
{
	// Si la distancia es mayor o igual a la distancia de reposo (pared)
	if (distance_mm >= DETECTION_IDLE_DISTANCE_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf("No hay caja presente (pared detectada a %u mm)\n", distance_mm);
		}
		return NO_BOX;
	}

	// Calculamos la altura de la caja como la diferencia con la pared
	uint8_t altura_mm = DETECTION_IDLE_DISTANCE_MM - distance_mm;

	if (altura_mm < DETECTION_THRESHOLD_MIN_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja demasiado baja (%u mm), descartada\n", altura_mm);
		}
		return BOX_DISCARDED;
	}

	// Comparamos contra los rangos definidos
	if (altura_mm >= SystemSorter->box_ranges->box_size_a.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_a.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo A (%u mm)\n", altura_mm);
		}
		return BOX_SIZE_A;
	}
	else if (altura_mm >= SystemSorter->box_ranges->box_size_b.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_b.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo B (%u mm)\n", altura_mm);
		}
		return BOX_SIZE_B;
	}
	else if (altura_mm >= SystemSorter->box_ranges->box_size_c.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_c.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo C (%u mm)\n", altura_mm);
		}
		return BOX_SIZE_C;
	}

	// Si no entró en ningún rango, se descarta
	if (DEBUG_FLAGS_SORTER) {
		printf("Caja fuera de rango (%u mm), descartada\n", altura_mm);
	}
	return BOX_DISCARDED;
}



void ultraSensorTask(Ultrasonic_Detector_t* ultraDetector, sorter_system_t * sorter)
{
	// 1. Si está habilitado, el trigger está permitido y hay solicitud de emisión
	if (ULTRASONIC_ENABLE && ultraDetector->sensor->TRIGGER_ALLOWED && EMIT_TRIGGER) //usamos -> para acceder a los atributos, porque lo que recibimos es un puntero
	{
		if (ultrasonic_start(ultraDetector->sensor))
		{
			if (DEBUG_FLAGS)
			{
				printf("InitHCSR04\n");
			}

			ULTRASONIC_ENABLE = 0;     // Desactivar para esperar fin de medición
			EMIT_TRIGGER = 0;          // Limpiar bandera de emisión
			ultraDetector->sensor->TRIGGER_ALLOWED = 0;  // Inhabilitar trigger hasta que se permita de nuevo
		}
		else
		{
			if (DEBUG_FLAGS)
			{
				printf("ErrorInitHCSR04\n");
			}
			EMIT_FAILED = 1;
			EMIT_TRIGGER = 0;
		}
	}

	// 2. Actualiza máquina de estados del sensor
	ultrasonic_update(ultraDetector->sensor);

	// 3. Si la medición finalizó exitosamente
	if (ultraDetector->sensor->state == ULTRA_DONE && ultraDetector->sensor->NEW_RESULT)
	{
		//printf("HCSR04 Dist[mm] %lu\n", (uint32_t)ultrasonic_get_distance(ultraDetector->sensor));
		ultraDetector->sensor->NEW_RESULT = 0;

		box_type_t tipo;

		// Si la distancia es mayor al límite de zona activa -> no hay caja
		if (ultraDetector->sensor->distance_mm > DETECTION_IDLE_DISTANCE_MM)
		{
			tipo = NO_BOX;
			ultraDetector->flags.bitmap.bit0 = 1; // ZONE_ULTRA_CLEAR

			// Si estaba esperando que se libere, volvemos a IDLE
			if (NIBBLEH_GET_STATE(ultraDetector->flags) == ULTRADET_SENSOR_WAITING_CLEAR)
			{
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_IDLE);
				if (DEBUG_FLAGS_SORTER) {
					printf("Sensor liberado de nuevo\n");
				}
			}

			NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_IDLE); // Momentáneo, se puede sacar si no se requiere más
		}
		else
		{
			// Si el sensor ya está libre (estado IDLE), se procede a clasificar la caja
			if (NIBBLEH_GET_STATE(ultraDetector->flags) == ULTRADET_SENSOR_IDLE) {
				// Hay algo en la zona
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_DETECTING);
				tipo = classify_box(ultraDetector->sensor->distance_mm, sorter);

				if (tipo != NO_BOX)
				{
					box_t nueva_caja;
					nueva_caja.height_mm = ultraDetector->sensor->distance_mm;
					nueva_caja.flags.byte = 0;
					nueva_caja.flags.nibbles.bitH = tipo; // Guarda tipo en nibble alto
					nueva_caja.state = BOX_MEASURED;

					// Debug de tipo
					switch (tipo)
					{
						case BOX_SIZE_A: printf("BOX_SIZE_A\n"); break;
						case BOX_SIZE_B: printf("BOX_SIZE_B\n"); break;
						case BOX_SIZE_C: printf("BOX_SIZE_C\n"); break;
						case BOX_DISCARDED: printf("BOX_DISCARDED\n"); break;
						default: printf("Tipo no reconocido\n"); break;
					}

					// Estadísticas
					if (tipo == BOX_DISCARDED)
					{
						nueva_caja.flags.bitmap.bit2 = 1; // DISCARDED
						sorter->stats.total_discarded++;
					}
					else
					{
						sorter->stats.total_selected++;
					}

					sorter->stats.total_measured++;
					sorter->stats.count_by_type[tipo]++;

					// TODO: guardar nueva_caja en buffer de cajas, si implementás uno
				}

				// Cambio de estado para esperar que se libere nuevamente
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_WAITING_CLEAR);
			}
		}

		ultrasonic_init_flags(ultraDetector->sensor); // Reinicia las flags del sensor
		ultraDetector->sensor->state = ULTRA_IDLE;   // Reinicia el estado del sensor
		ULTRASONIC_ENABLE = 1; // Habilita nueva medición
	}


	// 4. Si pasó el tiempo de espera y no hubo ECHO
	if (VEINTEMS_PASSED)
	{
		if (DEBUG_FLAGS)
		{
			printf("HCSR04 perdio ECHO\n");
		}
		VEINTEMS_PASSED = 0;
		WAITING_ECHO = 0;

		ultraDetector->sensor->TIMEDOUT = 1;
		ultraDetector->sensor->state = ULTRA_TIMEOUT;

		if (ultrasonic_timeout_clear(ultraDetector->sensor, DEBUG_FLAGS ? true : false) && DEBUG_FLAGS)
		{
			printf("LIB DEBUG - HCSR04 TMDOUT Cleared\n");
		}

		ULTRASONIC_ENABLE = 1;
	}
}

void irSensorsTask(sorter_system_t * sorter){
	if(IR_READ){
		printf("IR A: %u\n", IR_A.ADCConvertedValue);
		printf("IR B: %u\n", IR_B.ADCConvertedValue);
		IR_READ = 0;
	}
	if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_A.flags, TCRT_NEW_VALUE)){ //Cada 20 ms se activa
		CLEAR_FLAG(IR_A.flags, TCRT_NEW_VALUE);
		tcrt_read();
	}
}

void servosTask(){
	// SERVO A
	if (!IS_FLAG_SET(servoA.flags, SERVO_PUSH) && !IS_FLAG_SET(servoA.flags, SERVO_RESET)) {
		// Ya terminó su ciclo de activación ? limpiar orden externa
		CLEAR_FLAG(salidaA.flags, OUTPUT_PUSH);
		} else if (IS_FLAG_SET(salidaA.flags, OUTPUT_PUSH) && !IS_FLAG_SET(servoA.flags, SERVO_PUSH)) {
		// Solo setea si no se activó aún el pulso
		SET_FLAG(servoA.flags, SERVO_PUSH);
		SET_FLAG(servoA.flags, SERVO_RESET);
	}

	// SERVO B
	if (!IS_FLAG_SET(servoB.flags, SERVO_PUSH) && !IS_FLAG_SET(servoB.flags, SERVO_RESET)) {
		CLEAR_FLAG(salidaB.flags, OUTPUT_PUSH);
		} else if (IS_FLAG_SET(salidaB.flags, OUTPUT_PUSH) && !IS_FLAG_SET(servoB.flags, SERVO_PUSH)) {
		SET_FLAG(servoB.flags, SERVO_PUSH);
		SET_FLAG(servoB.flags, SERVO_RESET);
	}

	// SERVO C
	if (!IS_FLAG_SET(servoC.flags, SERVO_PUSH) && !IS_FLAG_SET(servoC.flags, SERVO_RESET)) {
		CLEAR_FLAG(salidaC.flags, OUTPUT_PUSH);
		} else if (IS_FLAG_SET(salidaC.flags, OUTPUT_PUSH) && !IS_FLAG_SET(servoC.flags, SERVO_PUSH)) {
		SET_FLAG(servoC.flags, SERVO_PUSH);
		SET_FLAG(servoC.flags, SERVO_RESET);
	}
}
