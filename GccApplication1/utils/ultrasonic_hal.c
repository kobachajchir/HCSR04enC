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

static volatile uint32_t g_timeoutTarget_us = 0;  // tiempo límite (en µs)

// -------------------------------------------------------------------------
// IMPLEMENTACIÓN DE LAS FUNCIONES HAL
// -------------------------------------------------------------------------

void ultrasonic_hal_trigger_setHigh()
{
    // Pone el pin TRIGGER en HIGH
    TRIGGER_PORT |= (1 << TRIGGER_PIN);
}

void ultrasonic_hal_trigger_setLow()
{
    // Pone el pin TRIGGER en LOW
    TRIGGER_PORT &= ~(1 << TRIGGER_PIN);
}

void ultrasonic_hal_setTrigger_timeout(uint32_t microseconds)
{
    return; //Esto ya lo hace el timer2 en su ciclo de CTC
}

void ultrasonic_hal_echo_timeout(ultrasonic_t *us)
{
	us->state = ULTRA_TIMEOUT;
}

void ultrasonic_hal_echo_setRisingEdge()
{
    // Set bit ICES1 = 1 => captura en flanco ascendente
    TCCR1B |= (1 << ICES1);
}

void ultrasonic_hal_echo_setFallingEdge()
{
    // ICES1 = 0 => captura en flanco descendente
    TCCR1B &= ~(1 << ICES1);
}