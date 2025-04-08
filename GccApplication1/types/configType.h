/**
 * @file configType.h
 * @brief Define la estructura de configuración del sistema.
 *
 * Contiene los tipos de caja asignados a cada salida y un campo de verificación (checksum)
 * para validar la integridad de la configuración almacenada en la EEPROM.
 *
 * @author kobac
 * @date 4/4/2025
 */

#ifndef CONFIGTYPE_H_
#define CONFIGTYPE_H_

#include "boxTypes.h"

/**
 * @struct Config_t
 * @brief Estructura que representa la configuración del sistema de clasificación.
 *
 * Contiene la configuración de tipos de caja asignados a las salidas del sistema
 * y un checksum simple para validar la integridad de los datos cuando se leen desde EEPROM.
 */
typedef struct {
	box_type_t salidaA;  /**< Tipo de caja asignado a la salida A. */
	box_type_t salidaB;  /**< Tipo de caja asignado a la salida B. */
	box_type_t salidaC;  /**< Tipo de caja asignado a la salida C. */
	uint8_t checksum;    /**< Checksum de verificación de la configuración. */
} Config_t;

#endif /* CONFIGTYPE_H_ */
