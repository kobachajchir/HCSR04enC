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

/**
 * @brief Inicializa un servo espec�fico.
 *
 * Asigna valores iniciales de �ngulo, pin, y estado. Habilita el servo.
 *
 * @param [in,out] servo Puntero a la estructura del servo a inicializar.
 * @param [in] index �ndice del servo en el arreglo global.
 * @param [in] pin Pin de salida asociado al servo.
 * @param [in] startAngle �ngulo inicial en grados (0 a 180).
 */
void initServo(volatile servo_t* servo, uint8_t index, uint8_t pin, uint8_t startAngle){
	servo->flags.byte = 0;
	servo->pulse_us = calculate_angle_pulseUs(startAngle);
	servo->pin = pin;
	servo->angle = startAngle;
	servo->state_time = 0;
	SET_FLAG(servo->flags, SERVO_ENABLE);
}

/**
 * @brief Convierte un �ngulo en microsegundos de pulso.
 *
 * Realiza un escalado lineal entre `SERVO_MIN_PULSE` y `SERVO_MAX_PULSE`.
 *
 * @param [in] angle �ngulo deseado (0�180 grados).
 *
 * @retval Duraci�n del pulso en microsegundos correspondiente al �ngulo.
 */
uint16_t calculate_angle_pulseUs(uint8_t angle){
	if (angle > 180) angle = 180;
	uint16_t min_ticks = SERVO_MIN_PULSE;
	uint16_t max_ticks = SERVO_MAX_PULSE;
	return (uint16_t)(min_ticks + (((uint32_t)(max_ticks - min_ticks) * angle) / 180U));
}

/**
 * @brief Establece el �ngulo de un servo del arreglo global `servosArray`.
 *
 * Calcula y asigna la duraci�n del pulso correspondiente al �ngulo deseado.
 *
 * @param [in] index �ndice del servo (0 a NUM_OUTPUTS-1).
 * @param [in] angle �ngulo deseado (0�180 grados).
 */
void servo_set_angle(uint8_t index, uint8_t angle)
{
	if (index >= NUM_OUTPUTS) return;
	if (angle > 180) angle = 180;
	servosArray[index]->angle = angle;
	// Escalado lineal: 0� ? min_ticks, 180� ? max_ticks
	servosArray[index]->pulse_us = calculate_angle_pulseUs(angle);
}

