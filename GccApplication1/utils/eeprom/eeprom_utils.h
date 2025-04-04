/*
 * eeprom_utils.h
 *
 * Created: 4/4/2025 5:00:37 PM
 *  Author: kobac
 */ 


#ifndef EEPROM_UTILS_H_
#define EEPROM_UTILS_H_

#include "../../types/configType.h"
#include <stdbool.h>

void saveConfiguration(Config_t *config);
void loadConfiguration(Config_t *config);
bool existConfig(void);


#endif /* EEPROM_UTILS_H_ */