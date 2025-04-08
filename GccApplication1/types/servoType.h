/**
 * @file servoType.h
 * @brief Definición del tipo de dato para el control de servomotores.
 *
 * Contiene la estructura que representa un servomotor y los flags asociados a su control.
 * Incluye ángulo, duración del pulso, tiempo de estado y control por pines.
 *
 * @author kobac
 * @date 28-Mar-25
 */

#ifndef SERVOTYPE_H_
#define SERVOTYPE_H_

#include "bitmapType.h"

/// @brief Flag para habilitar el servomotor.
#define SERVO_ENABLE BIT0_MASK
/// @brief Flag para activar el empuje del servo (modo PUSH).
#define SERVO_PUSH BIT1_MASK
/// @brief Flag para solicitar el reinicio del servo a su ángulo de reposo.
#define SERVO_RESET BIT2_MASK
/// @brief Flag que indica si el servo debe moverse.
#define SERVO_MOVE BIT3_MASK

/**
 * @struct servo_t
 * @brief Representa un servomotor con su configuración y estado.
 *
 * Contiene información sobre el pin asignado, los flags de control, 
 * la duración del pulso (en microsegundos), el ángulo actual y el tiempo de estado.
 */
typedef struct {
	uint8_t pin;              /**< Pin asignado al servo */
	Byte_Flag_Struct flags;   /**< Flags de control del servo (habilitado, push, reset, etc.) */
	uint16_t pulse_us;        /**< Duración del pulso actual en microsegundos */
	uint8_t angle;            /**< Ángulo actual del servo (0–180 grados) */
	uint8_t state_time;       /**< Tiempo en que se mantiene en un estado activo (ej. en modo push) */
} servo_t;

#endif /* SERVOTYPE_H_ */