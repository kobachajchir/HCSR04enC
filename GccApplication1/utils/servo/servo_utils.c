/*
 * servo_utils.c
 *
 * Created: 23-Mar-25 9:41:38 PM
 *  Author: kobac
 */ 
#include <avr/io.h>
#include <stdio.h>
#include "./../../main.h"
#include "servo_utils.h"

void servoA_set_angle(uint8_t angle)
{
	if (angle > 180) {
		angle = 180;
	}
	// Convertir grados a ancho de pulso (entre MIN y MAX) usando 32 bits sin signo
	pulse_width_A = SERVO_MIN_PULSE +
	( ( (uint32_t)(SERVO_MAX_PULSE - SERVO_MIN_PULSE) * angle ) / 180U );
}