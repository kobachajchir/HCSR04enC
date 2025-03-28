/*
 * servo_utils.c
 *
 * Created: 23-Mar-25 9:41:38 PM
 *  Author: kobac
 */ 
#include <avr/io.h>
#include <stdio.h>
#include "./../../main.h"
#include "../../types/servoType.h"
#include "servo_utils.h"

void initServo(servo_t* servo, uint8_t index, uint8_t pin){
	printf("Init Servo %u\n", index);
	servo->flags.byte = 0;
	SET_FLAG(servo->flags, SERVO_ENABLE);
	servo->pulse_us = SERVO_START_PULSE;
	servo->pin = pin;
	servosArray[index] = servo;
}

void servo_set_angle(uint8_t index, uint8_t angle)
{
	if (index >= NUM_OUTPUTS) return;
	if (angle > 180) angle = 180;
	servosArray[index]->angle = angle;
	uint16_t min_ticks = SERVO_MIN_PULSE;
	uint16_t max_ticks = SERVO_MAX_PULSE;
	// Escalado lineal: 0º ? min_ticks, 180º ? max_ticks
	servosArray[index]->pulse_us = min_ticks + (((uint32_t)(max_ticks - min_ticks) * angle) / 180U);
}

void update_servos()
{
	for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
		PORTB |= (1 << servosArray[i]->pin); // Iniciar pulso
		if (IS_FLAG_SET(servosArray[i]->flags, OUTPUT_PUSH)) {
			servosArray[i]->pulse_us = SERVO_MAX_PULSE; // Empuja
		} else {
			servosArray[i]->pulse_us = SERVO_START_PULSE; // Recto
		}
	}
}
