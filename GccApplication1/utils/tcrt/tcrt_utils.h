/*
 * tcrt_utils.h
 *
 * Created: 27-Mar-25 11:36:19 AM
 *  Author: kobac
 */ 


#ifndef TCRT_UTILS_H_
#define TCRT_UTILS_H_

#include "types/TCRTType.h"

void tcrt_init(void);
uint16_t tcrt_read_channel(uint8_t channel);
void TCRT_init_Handlers();

#endif /* TCRT_UTILS_H_ */