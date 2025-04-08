/**
 * @file TCRTType.h
 * @brief Definición del tipo de dato TCRT_t para sensores infrarrojos reflectivos.
 *
 * Este archivo define la estructura de un sensor TCRT (infrarrojo reflectivo),
 * incluyendo sus estados, flags, y variables utilizadas para la calibración,
 * lectura y detección de objetos mediante el ADC.
 *
 * @author kobac
 * @date 27-Mar-25
 */


#ifndef TCRTTYPE_H_
#define TCRTTYPE_H_

#include "bitmapType.h"

/**
 * @struct TCRT_t
 * @brief Representa un sensor TCRT (IR reflectivo) con filtrado y calibración.
 *
 * Contiene datos asociados a la lectura ADC, umbral, calibración, estado de lectura,
 * y otros parámetros relacionados con el procesamiento del sensor.
 */
typedef struct {
	Byte_Flag_Struct flags;        /**< Flags de estado y control (nibble H: estado, nibble L: banderas) */
	uint16_t ADCConvertedValue;    /**< Último valor promedio convertido del ADC (0–1023) */
	uint16_t threshold;            /**< Umbral de comparación para detección de objetos */
	uint16_t lastReading;          /**< Última lectura o acumulación de calibración */
	uint8_t channel;               /**< Canal ADC asignado al sensor */
	uint8_t pin;                   /**< Pin físico asociado al sensor */
	uint8_t calibrationCounter;    /**< Contador de muestras para calibración/filtrado */
	uint32_t filterAccumulator;    /**< Acumulador de muestras para filtrado */
	uint8_t hysteresis_percent;    /**< Porcentaje de histéresis aplicado sobre el threshold */
} TCRT_t;

/// @brief Bandera que indica que el sensor está habilitado.
#define TCRT_ENABLED BIT0_MASK
/// @brief Bandera que indica que se detectó flanco de subida (detección activa).
#define TCRT_EDGE_RISING BIT1_MASK
/// @brief Bandera que indica que el sensor está en proceso de calibración.
#define TCRT_CALIBRATING BIT2_MASK
/// @brief Bandera que indica que hay una nueva lectura disponible.
#define TCRT_NEW_VALUE BIT3_MASK

/// @brief Estado: el sensor está inactivo.
#define TCRT_STATUS_IDLE 0
/// @brief Estado: objeto detectado (lectura activa).
#define TCRT_READ 1
/// @brief Estado: objeto detectado y contado
#define TCRT_COUNTED 2
/// @brief Estado: sensor desactivado.
#define TCRT_DEACTIVATED 3

#endif /* TCRTTYPE_H */