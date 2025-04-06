/*
 * configType.h
 *
 * Created: 4/4/2025 4:59:51 PM
 *  Author: kobac
 */ 


#ifndef CONFIGTYPE_H_
#define CONFIGTYPE_H_

#include "boxTypes.h"

// Estructura de configuración
typedef struct {
	box_type_t salidaA;
	box_type_t salidaB;
	box_type_t salidaC;
	uint8_t checksum;  // Nuevo campo
} Config_t;

#endif /* CONFIGTYPE_H_ */