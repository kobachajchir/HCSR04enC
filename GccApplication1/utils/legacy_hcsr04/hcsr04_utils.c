/*
 * hcsr04_utils.c
 *
 * Created: 22-Mar-25 5:43:34 PM
 *  Author: kobac
 */ 
#include <avr/io.h>
#include <stdio.h>
#include "./../../main.h"
#include "hcsr04_utils.h"

void calculate_distance()
{
	// El tiempo de vuelo es la diferencia entre el tiempo final y el tiempo inicial
	uint32_t pulse_duration = echo_finish_time - echo_init_time;
	//Cada pulso es de 500ns y queremos en micro para que sea mas simple el calculo, deberiamos dividir en 2, pero ya dividimos en 200, entonces dividamos en 400
	// Calcula la distancia en milímetros
	distance_mm = (pulse_duration * 34) / 400;  // Multiplicamos por 34 y dividimos entre 400
	if (distance_mm >= DISTANCE_MIN_MM && distance_mm <= DISTANCE_MAX_MM) {
		printf("Distancia: %umm\n", distance_mm);
	}
	ovf_count = 0;
}