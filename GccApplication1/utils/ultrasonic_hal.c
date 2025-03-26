/*
 * ultrasonic_hal.c
 *
 * Created: 24-Mar-25 8:37:30 PM
 *  Author: kobac
 */ 

#include "ultrasonic.h"
#include "ultrasonic_hal.h"
#include "../main.h"
#include <avr/io.h>
#include <stddef.h>

static volatile uint32_t g_timeoutTarget_us = 0;  // tiempo l�mite (en �s)

// -------------------------------------------------------------------------
// IMPLEMENTACI�N DE LAS FUNCIONES HAL
// -------------------------------------------------------------------------

void ultrasonic_hal_trigger_setHigh(ultrasonic_t *us)
{
	TRIGGER_PORT |= (1 << TRIGGER_PIN);
}

void ultrasonic_hal_trigger_setLow(ultrasonic_t *us)
{
	TRIGGER_PORT &= ~(1 << TRIGGER_PIN);
}

void ultrasonic_hal_setTrigger_timeout(ultrasonic_t *us, uint32_t microseconds)
{
}

void ultrasonic_hal_echo_timeout(ultrasonic_t *us)
{
	us->TIMEDOUT = 1;
}

void ultrasonic_hal_echo_setRisingEdge(ultrasonic_t *us)
{
	TCCR1B |= (1 << ICES1);
}

void ultrasonic_hal_echo_setFallingEdge(ultrasonic_t *us)
{
	TCCR1B &= ~(1 << ICES1);
}