/*
 * tcrt_utils.h
 *
 * Created: 27-Mar-25 11:36:19 AM
 *  Author: kobac
 */ 


#ifndef TCRT_UTILS_H_
#define TCRT_UTILS_H_

#include "types/TCRTType.h"
#include <stdbool.h>

void tcrt_init(void);
uint16_t tcrt_read_channel(uint8_t channel);
bool detect_rising_edge(TCRT_t* sensor, uint8_t current_state)
void TCRT_init_Handlers();
void calibrateIRSensor(TCRT_t* sensor);
void tcrt_read(TCRT_t* sensor);

#endif /* TCRT_UTILS_H_ */