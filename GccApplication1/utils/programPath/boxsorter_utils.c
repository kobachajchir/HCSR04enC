/*
 * boxSorter.c
 *
 * Created: 26-Mar-25 9:34:08 AM
 *  Author: kobac
 */ 

#include "../../main.h"
#include "../../types/boxTypes.h"
#include "../../types/sorterSystemTypes.h"
#include "boxsorter_utils.h"
#include <stdio.h>
#include <stdbool.h>

void initSorter(sorter_system_t* SystemSorter){
	static box_height_range_matrix_t local_box_ranges = {
		.box_size_a = { BOX_A_MIN_MM, BOX_A_MAX_MM },
		.box_size_b = { BOX_B_MIN_MM, BOX_B_MAX_MM },
		.box_size_c = { BOX_C_MIN_MM, BOX_C_MAX_MM }
	};

	SystemSorter->box_ranges = &local_box_ranges;

	if(DEBUG_FLAGS_SORTER){
		printf("Matriz de dimensiones iniciada\n");
	}
}


box_type_t classify_box(uint8_t height_mm, sorter_system_t* SystemSorter)
{
	if (height_mm > DETECTION_IDLE_DISTANCE_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf("No hay caja presente\n");
		}
		return NO_BOX;
	}

	if (height_mm < DETECTION_THRESHOLD_MIN_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja fuera de rango (muy baja), descartada\n");
		}
		return BOX_DISCARDED;
	}

	if (height_mm >= SorterSystem.box_ranges->box_size_a.min_height_mm && height_mm <= SorterSystem.box_ranges->box_size_a.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo A\n");
		}
		return BOX_SIZE_A;
	}
	else if (height_mm >= SorterSystem.box_ranges->box_size_b.min_height_mm && height_mm <= SorterSystem.box_ranges->box_size_b.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo B\n");
		}
		return BOX_SIZE_B;
	}
	else if (height_mm >= SorterSystem.box_ranges->box_size_c.min_height_mm && height_mm <= SorterSystem.box_ranges->box_size_c.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo C\n");
		}
		return BOX_SIZE_C;
	}
	else {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja fuera de rango (descartada)\n");
		}
		return BOX_DISCARDED;
	}
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
		printf("HCSR04 Dist[mm] %lu\n", (uint32_t)ultrasonic_get_distance(ultraDetector->sensor));
		ultraDetector->sensor->NEW_RESULT = 0;

		box_type_t tipo;

		// Si la distancia es mayor al límite de zona activa ? no hay caja
		if (ultraDetector->sensor->distance_mm > DETECTION_IDLE_DISTANCE_MM)
		{
			tipo = NO_BOX;
			ultraDetector->flags.bitmap.bit0 = 1; // ZONE_ULTRA_CLEAR

			// Si estaba esperando que se libere, volvemos a IDLE
			if (NIBBLEH_GET_STATE((*ultraDetector)) == SENSOR_WAITING_CLEAR)
			{
				NIBBLEH_SET_STATE((*ultraDetector), SENSOR_IDLE);
				if (DEBUG_FLAGS_SORTER) {
					printf("Sensor liberado de nuevo\n");
				}
			}

			NIBBLEH_SET_STATE((*ultraDetector), SENSOR_IDLE);
		}
		else
		{
			// Hay algo en la zona
			NIBBLEH_SET_STATE((*ultraDetector), SENSOR_DETECTING);
			tipo = classify_box(ultraDetector->sensor->distance_mm, &sorter->box_ranges->box_size_a);

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

// 				// Estadísticas
// 				if (tipo == BOX_DISCARDED)
// 				{
// 					nueva_caja.flags.bitmap.bit2 = 1; // DISCARDED
// 					sorter->stats.total_discarded++;
// 				}
// 				else
// 				{
// 					sorter->stats.total_selected++;
// 				}
// 
// 				sorter->stats.total_measured++;
// 				sorter->stats.count_by_type[tipo]++;
// 
// 				// TODO: guardar nueva_caja en buffer de cajas, si implementás uno
			}

			NIBBLEH_SET_STATE((*ultraDetector), SENSOR_WAITING_CLEAR);
		}

		ultrasonic_init_flags(ultraDetector->sensor);
		ultraDetector->sensor->state = ULTRA_IDLE;
		ULTRASONIC_ENABLE = 1;
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
