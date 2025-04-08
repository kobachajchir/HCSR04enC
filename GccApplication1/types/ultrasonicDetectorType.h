/**
 * @file ultrasonicDetectorType.h
 * @brief Definici�n de la estructura y macros para el detector ultras�nico compuesto.
 *
 * Este archivo define la estructura `Ultrasonic_Detector_t`, que combina un sensor ultras�nico
 * con un sensor infrarrojo (TCRT), as� como los flags y estados utilizados para el control
 * del proceso de detecci�n de cajas mediante ultrasonido.
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

/// @brief Flag que indica que la zona del sensor ultras�nico est� despejada.
#define ULTRADET_ZONE_ULTRA_CLEAR BIT0_MASK
/// @brief Flag que indica que el sensor IR U est� detectando una caja.
#define ULTRADET_ZONE_TRCT_U_DETECTING BIT1_MASK
/*#define DISCARDED flags.bitmap.bit2;
#define DISCARDED flags.bitmap.bit3;*/
// NO AGREGAR MAS -> Nibble H lo usamos para el tipo

/// @brief Estado del sensor: en reposo (esperando una detecci�n).
#define	ULTRADET_SENSOR_IDLE 0
/// @brief Estado del sensor: detectando una caja.
#define	ULTRADET_SENSOR_DETECTING 1
/// @brief Estado del sensor: esperando que se libere la zona (la caja se retire).
#define	ULTRADET_SENSOR_WAITING_CLEAR 2

/**
 * @struct Ultrasonic_Detector_t
 * @brief Estructura que representa un sistema de detecci�n basado en sensor ultras�nico e IR.
 *
 * Contiene punteros a los sensores usados para la detecci�n de presencia y clasificaci�n
 * de cajas, as� como un conjunto de banderas de estado y control.
 */
typedef struct{
	ultrasonic_t* sensor;        /**< Puntero al sensor ultras�nico (HCSR04) */
	TCRT_t* sensor_IR;           /**< Puntero al sensor infrarrojo (TCRT) */
	Byte_Flag_Struct flags;      /**< Flags: nibble H para estado, nibble L para flags de zona */
} Ultrasonic_Detector_t;

#endif /* ULTRASONICDETECTORTYPE_H_ */