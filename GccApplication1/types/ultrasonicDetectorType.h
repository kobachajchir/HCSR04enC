/**
 * @file ultrasonicDetectorType.h
 * @brief Definición de la estructura y macros para el detector ultrasónico compuesto.
 *
 * Este archivo define la estructura `Ultrasonic_Detector_t`, que combina un sensor ultrasónico
 * con un sensor infrarrojo (TCRT), así como los flags y estados utilizados para el control
 * del proceso de detección de cajas mediante ultrasonido.
 *
 * @author kobac
 * @date 26-Mar-25
 */


#ifndef ULTRASONICDETECTORTYPE_H_
#define ULTRASONICDETECTORTYPE_H_

#include "ultrasonic.h"
#include "ultrasonic_hal.h"
#include "bitmapType.h"
#include "TCRTType.h"

/// @brief Flag que indica que la zona del sensor ultrasónico está despejada.
#define ULTRADET_ZONE_ULTRA_CLEAR BIT0_MASK
/// @brief Flag que indica que el sensor IR U está detectando una caja.
#define ULTRADET_ZONE_TRCT_U_DETECTING BIT1_MASK
/*#define DISCARDED flags.bitmap.bit2;
#define DISCARDED flags.bitmap.bit3;*/
// NO AGREGAR MAS -> Nibble H lo usamos para el tipo

/// @brief Estado del sensor: en reposo (esperando una detección).
#define	ULTRADET_SENSOR_IDLE 0
/// @brief Estado del sensor: detectando una caja.
#define	ULTRADET_SENSOR_DETECTING 1
/// @brief Estado del sensor: esperando que se libere la zona (la caja se retire).
#define	ULTRADET_SENSOR_WAITING_CLEAR 2

/**
 * @struct Ultrasonic_Detector_t
 * @brief Estructura que representa un sistema de detección basado en sensor ultrasónico e IR.
 *
 * Contiene punteros a los sensores usados para la detección de presencia y clasificación
 * de cajas, así como un conjunto de banderas de estado y control.
 */
typedef struct{
	ultrasonic_t* sensor;        /**< Puntero al sensor ultrasónico (HCSR04) */
	TCRT_t* sensor_IR;           /**< Puntero al sensor infrarrojo (TCRT) */
	Byte_Flag_Struct flags;      /**< Flags: nibble H para estado, nibble L para flags de zona */
} Ultrasonic_Detector_t;

#endif /* ULTRASONICDETECTORTYPE_H_ */