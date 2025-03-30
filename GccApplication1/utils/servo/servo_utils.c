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

void initServo(volatile servo_t* servo, uint8_t index, uint8_t pin, uint8_t startAngle){
	printf("Init Servo %u\n", index);
	servo->flags.byte = 0;
	servo->pulse_us = calculate_angle_pulseUs(startAngle);
	servo->pin = pin;
	servo->angle = startAngle;
	servo->state_time = 0;
	SET_FLAG(servo->flags, SERVO_ENABLE);
}

uint16_t calculate_angle_pulseUs(uint8_t angle){
	if (angle > 180) angle = 180;
	uint16_t min_ticks = SERVO_MIN_PULSE;
	uint16_t max_ticks = SERVO_MAX_PULSE;
	return (uint16_t)(min_ticks + (((uint32_t)(max_ticks - min_ticks) * angle) / 180U));
}

void servo_set_angle(uint8_t index, uint8_t angle)
{
	if (index >= NUM_OUTPUTS) return;
	if (angle > 180) angle = 180;
	servosArray[index]->angle = angle;
	// Escalado lineal: 0º ? min_ticks, 180º ? max_ticks
	servosArray[index]->pulse_us = calculate_angle_pulseUs(angle);
}

