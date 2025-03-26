/*
 * boxSorter.c
 *
 * Created: 26-Mar-25 9:34:08 AM
 *  Author: kobac
 */ 

#include "../../main.h"
#include "../../types/boxTypes.h"
#include "boxsorter_utils.h"
#include <stdio.h>

box_type_t classify_box(uint16_t height_mm, box_height_range_t *ranges)
{
	if (height_mm >= ranges[0].min_height_mm && height_mm <= ranges[0].max_height_mm)
	return BOX_SIZE_A;
	else if (height_mm >= ranges[1].min_height_mm && height_mm <= ranges[1].max_height_mm)
	return BOX_SIZE_B;
	else if (height_mm >= ranges[2].min_height_mm && height_mm <= ranges[2].max_height_mm)
	return BOX_SIZE_C;
	else
	return BOX_DISCARDED;
}

void ultraSensorTask(ultrasonic_t* ultraSensor)
{
	// 1. Si está habilitado, el trigger está permitido y hay solicitud de emisión
	if (ULTRASONIC_ENABLE && ultraSensor->TRIGGER_ALLOWED && EMIT_TRIGGER) //usamos -> para acceder a los atributos, porque lo que recibimos es un puntero
	{
		if (ultrasonic_start(ultraSensor))
		{
			if (DEBUG_FLAGS)
			{
				printf("InitHCSR04\n");
			}

			ULTRASONIC_ENABLE = 0;     // Desactivar para esperar fin de medición
			EMIT_TRIGGER = 0;          // Limpiar bandera de emisión
			ultraSensor->TRIGGER_ALLOWED = 0;  // Inhabilitar trigger hasta que se permita de nuevo
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
	ultrasonic_update(ultraSensor);

	// 3. Si la medición finalizó exitosamente
	if (ultraSensor->state == ULTRA_DONE && ultraSensor->NEW_RESULT)
	{
		printf("HCSR04 Dist[mm] %lu\n", (uint32_t)ultrasonic_get_distance(ultraSensor));
		ultraSensor->NEW_RESULT = 0;
		ultrasonic_init_flags(ultraSensor);     // Limpia flags internos
		ultraSensor->state = ULTRA_IDLE;
		ULTRASONIC_ENABLE = 1;  // Habilita nueva medición
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

		ultraSensor->TIMEDOUT = 1;
		ultraSensor->state = ULTRA_TIMEOUT;

		if (ultrasonic_timeout_clear(ultraSensor, DEBUG_FLAGS ? true : false) && DEBUG_FLAGS)
		{
			printf("LIB DEBUG - HCSR04 TMDOUT Cleared\n");
		}

		ULTRASONIC_ENABLE = 1;
	}
}
