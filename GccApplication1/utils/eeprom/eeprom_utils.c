/*
 * eeprom_utils.c
 *
 * Created: 4/4/2025 5:00:54 PM
 *  Author: kobac
 */ 

#include "eeprom_utils.h"
#include "../../main.h"
#include <stdbool.h>

// Guarda la configuración desde RAM en la EEPROM
void saveConfigurationRAM(const Config_t *config) {
	Config_t temp = *config; // Copia local en RAM
	temp.checksum = calculate_config_checksum(&temp);
	eeprom_update_block((const void*)&temp, (void*)&eepromConfig, sizeof(Config_t));
	printf_P(PSTR("Config guardada en EEPROM\n"));
}

// Función para calcular el checksum de la configuración
uint8_t calculate_config_checksum(Config_t* config) {
	// Simple XOR de los campos de salida
	return config->salidaA ^ config->salidaB ^ config->salidaC;
}

void loadConfigurationRAM(Config_t *config) {
	if (config == NULL) return;
	eeprom_read_block((void *)config, (const void *)&eepromConfig, sizeof(Config_t));
	printf_P(PSTR("Config cargada desde EEPROM\n"));
}


// Verifica si existe una configuración válida en la EEPROM
bool existConfig(void) {
	Config_t temp;
	eeprom_read_block((void*)&temp, (const void *)&eepromConfig, sizeof(Config_t));
	
	// Si todas las salidas son 0xFF se asume que no hay configuración seteada.
	if (temp.salidaA == 0xFF && temp.salidaB == 0xFF && temp.salidaC == 0xFF) {
		return false;
	}
	
	uint8_t expected = calculate_config_checksum(&temp);
	if (temp.checksum != expected) {
		printf_P(PSTR("Checksum EEPROM invalido\n"));
		return false;
	}
	
	printf_P(PSTR("Configuración válida en EEPROM\n"));
	return true;
}