/**
 * @file outputType.h
 * @brief Define el tipo de dato para representar una salida del sistema de clasificación.
 *
 * Incluye pines asociados al sensor y actuador, flags de estado y tipo de caja asignado.
 *
 * @author kobac
 * @date 26-Mar-25
 */

#ifndef OUTPUTTYPE_H_
#define OUTPUTTYPE_H_

#include "boxTypes.h"
#include "bitmapType.h"
#include "boxTypes.h"

/**
 * @def OUTPUT_BUSY
 * @brief Flag que indica que la salida está ocupada procesando una caja.
 */
#define OUTPUT_BUSY BIT0_MASK
/**
 * @def OUTPUT_PUSH
 * @brief Flag que indica que el actuador debe empujar una caja.
 */
#define OUTPUT_PUSH BIT1_MASK
/**
 * @def OUTPUT_RESET
 * @brief Flag que indica que el actuador debe volver a su posición de reposo.
 */
#define OUTPUT_RESET BIT2_MASK
/**
 * @def OUTPUT_READY
 * @brief Flag que indica que la salida está lista para recibir una nueva caja.
 */
#define OUTPUT_READY BIT3_MASK
//Solo NIBBLE L para flags

/// Definiciones por defecto del tipo de caja por salida
#define OUTPUT_A_DEFAULT_BOX_TYPE BOX_SIZE_A
#define OUTPUT_B_DEFAULT_BOX_TYPE BOX_SIZE_B
#define OUTPUT_C_DEFAULT_BOX_TYPE BOX_SIZE_C

/**
 * @struct output_t
 * @brief Estructura que representa una salida del sistema de clasificación.
 *
 * Contiene información sobre el pin del sensor, pin del actuador, flags de estado
 * y el tipo de caja asignado a la salida.
 */
typedef struct {
	uint8_t sensor_pin;         /**< Pin digital conectado al sensor TCRT de la salida. */
	uint8_t actuator_pin;       /**< Pin digital conectado al actuador (servo). */
	Byte_Flag_Struct flags;     /**< Flags de control: nibble bajo para estado, nibble alto opcional. */
	box_type_t boxType;         /**< Tipo de caja que esta salida está configurada para recibir. */
} output_t;

#endif /* OUTPUTTYPE_H_ */