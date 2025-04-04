/*
 * eeprom_utils.c
 *
 * Created: 4/4/2025 5:00:54 PM
 *  Author: kobac
 */ 

#include "eeprom_utils.h"
#include "../../main.h"
#include <stdbool.h>

// Función para guardar la configuración en la EEPROM
void saveConfiguration(Config_t *config) {
	// Usa eeprom_update_block para escribir solo si ha habido cambios
	printf("Save config\n");
	eeprom_update_block((const void*)config, (void*)&eepromConfig, sizeof(Config_t));
}

// Función para cargar la configuración de la EEPROM
void loadConfiguration(Config_t *config) {
	printf("Load config\n");
	eeprom_read_block((void*)config, (const void*)&eepromConfig, sizeof(Config_t));
}

bool existConfig(void) {
	Config_t temp;
	eeprom_read_block((void*)&temp, (const void*)&eepromConfig, sizeof(Config_t));
	// Si todos los bytes son 0xFF, se asume que no hay configuración seteada.
	if (temp.salidaA == 0xFF && temp.salidaB == 0xFF && temp.salidaC == 0xFF) {
		return false;
	}else{
		printf("Exists config\n");
	}
	return true;
}
