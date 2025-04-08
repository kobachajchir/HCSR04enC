/**
 * @file TCRTType.h
 * @brief Definici�n del tipo de dato TCRT_t para sensores infrarrojos reflectivos.
 *
 * Este archivo define la estructura de un sensor TCRT (infrarrojo reflectivo),
 * incluyendo sus estados, flags, y variables utilizadas para la calibraci�n,
 * lectura y detecci�n de objetos mediante el ADC.
 *
 * @author kobac
 * @date 27-Mar-25
 */


#ifndef TCRTTYPE_H_
#define TCRTTYPE_H_

#include "bitmapType.h"

/**
 * @struct TCRT_t
 * @brief Representa un sensor TCRT (IR reflectivo) con filtrado y calibraci�n.
 *
 * Contiene datos asociados a la lectura ADC, umbral, calibraci�n, estado de lectura,
 * y otros par�metros relacionados con el procesamiento del sensor.
 */
typedef struct {
	Byte_Flag_Struct flags;        /**< Flags de estado y control (nibble H: estado, nibble L: banderas) */
	uint16_t ADCConvertedValue;    /**< �ltimo valor promedio convertido del ADC (0�1023) */
	uint16_t threshold;            /**< Umbral de comparaci�n para detecci�n de objetos */
	uint16_t lastReading;          /**< �ltima lectura o acumulaci�n de calibraci�n */
	uint8_t channel;               /**< Canal ADC asignado al sensor */
	uint8_t pin;                   /**< Pin f�sico asociado al sensor */
	uint8_t calibrationCounter;    /**< Contador de muestras para calibraci�n/filtrado */
	uint32_t filterAccumulator;    /**< Acumulador de muestras para filtrado */
	uint8_t hysteresis_percent;    /**< Porcentaje de hist�resis aplicado sobre el threshold */
} TCRT_t;

/// @brief Bandera que indica que el sensor est� habilitado.
#define TCRT_ENABLED BIT0_MASK
/// @brief Bandera que indica que se detect� flanco de subida (detecci�n activa).
#define TCRT_EDGE_RISING BIT1_MASK
/// @brief Bandera que indica que el sensor est� en proceso de calibraci�n.
#define TCRT_CALIBRATING BIT2_MASK
/// @brief Bandera que indica que hay una nueva lectura disponible.
#define TCRT_NEW_VALUE BIT3_MASK

/// @brief Estado: el sensor est� inactivo.
#define TCRT_STATUS_IDLE 0
/// @brief Estado: objeto detectado (lectura activa).
#define TCRT_READ 1
/// @brief Estado: objeto detectado y contado
#define TCRT_COUNTED 2
/// @brief Estado: sensor desactivado.
#define TCRT_DEACTIVATED 3

#endif /* TCRTTYPE_H */