/*
 * TCRTType.c
 *
 * Created: 27-Mar-25 11:44:20 AM
 *  Author: kobac
 */ 

#ifndef TCRTTYPE_H_
#define TCRTTYPE_H_

#include "bitmapType.h"

typedef struct {
	Byte_Flag_Struct flags; //Aca tenemos el estado en el nibble H, y las banderas en el nibble L
	uint16_t ADCConvertedValue; //Resolucion 10 bits 0 - 1023
	uint16_t threshold;
	uint16_t lastReading;
	uint8_t channel; //Canal TCRT_X_CHANNEL
	uint8_t pin;
	uint8_t calibrationCounter;
}TCRT_t;

#define TCRT_ENABLED BIT0_MASK
#define TCRT_EDGE_RISING BIT1_MASK
#define TCRT_CALIBRATING BIT2_MASK
#define TCRT_NEW_VALUE BIT3_MASK

#define TCRT_STATUS_IDLE 0
#define TCRT_READ 1
#define TCRT_COUNTED 2
#define TCRT_DEACTIVATED 3

#endif /* TCRTTYPE_H */