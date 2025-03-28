/*
 * servoType.h
 *
 * Created: 28-Mar-25 10:36:02 AM
 *  Author: kobac
 */ 


#ifndef SERVOTYPE_H_
#define SERVOTYPE_H_

#include "bitmapType.h"

#define SERVO_ENABLE BIT0_MASK
#define SERVO_PUSH BIT1_MASK
#define SERVO_RESET BIT2_MASK

typedef struct {
	uint8_t pin;
	Byte_Flag_Struct flags;
	uint16_t pulse_us;
	uint8_t angle;
} servo_t;

#endif /* SERVOTYPE_H_ */