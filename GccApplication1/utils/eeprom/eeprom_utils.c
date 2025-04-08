/*
 * eeprom_utils.c
 *
 * Created: 4/4/2025 5:00:54 PM
 *  Author: kobac
 */ 

#include "eeprom_utils.h"
#include "../../main.h"
#include <stdbool.h>

/**
 * @brief Guarda la configuraci�n en EEPROM desde una copia en RAM.
 *
 * Copia la estructura recibida, calcula su checksum y la guarda usando `eeprom_update_block`.
 *
 * @param [in] config Puntero a la configuraci�n en RAM que se desea guardar.
 */
void saveConfigurationRAM(const Config_t *config) {
	Config_t temp = *config; // Copia local en RAM
	temp.checksum = calculate_config_checksum(&temp);
	eeprom_update_block((const void*)&temp, (void*)&eepromConfig, sizeof(Config_t));
	if(DEBUG_FLAGS_EEPROM){
		printf_P(PSTR("Config guardada en EEPROM\n"));
	}
}

/**
 * @brief Calcula el checksum de una estructura de configuraci�n.
 *
 * Realiza una operaci�n XOR entre los valores de salidaA, salidaB y salidaC.
 *
 * @param [in] config Puntero a la configuraci�n a verificar.
 * @retval Checksum calculado como XOR de los campos de salida.
 */
uint8_t calculate_config_checksum(Config_t* config) {
	// Simple XOR de los campos de salida
	return config->salidaA ^ config->salidaB ^ config->salidaC;
}

/**
 * @brief Carga la configuraci�n desde EEPROM a una estructura en RAM.
 *
 * Si el puntero es v�lido, lee el bloque completo desde EEPROM.
 *
 * @param [out] config Puntero a la estructura donde se cargar� la configuraci�n.
 */
void loadConfigurationRAM(Config_t *config) {
	if (config == NULL) return;
	eeprom_read_block((void *)config, (const void *)&eepromConfig, sizeof(Config_t));
	if(DEBUG_FLAGS_EEPROM){
		printf_P(PSTR("Config cargada: Salida 0 = %c, Salida 1 = %c, Salida 2 = %c\n"),
		config->salidaA + 'A', config->salidaB + 'A', config->salidaC + 'A');
	}
}

/**
 * @brief Verifica si existe una configuraci�n v�lida en la EEPROM.
 *
 * Comprueba si los campos son distintos de 0xFF y si el checksum es correcto.
 *
 * @retval true Si la configuraci�n es v�lida.
 * @retval false Si la configuraci�n no est� presente o el checksum no coincide.
 */
bool existConfig(void) {
	Config_t temp;
	eeprom_read_block((void*)&temp, (const void *)&eepromConfig, sizeof(Config_t));
	
	// Si todas las salidas son 0xFF se asume que no hay configuraci�n seteada.
	if (temp.salidaA == 0xFF && temp.salidaB == 0xFF && temp.salidaC == 0xFF) {
		return false;
	}
	
	uint8_t expected = calculate_config_checksum(&temp);
	if (temp.checksum != expected) {
		if(DEBUG_FLAGS_EEPROM){
			printf_P(PSTR("Checksum EEPROM invalido\n"));
		}
		return false;
	}
	
	if(DEBUG_FLAGS_EEPROM){
		printf_P(PSTR("Configuraci�n v�lida en EEPROM\n"));
	}
	return true;
}