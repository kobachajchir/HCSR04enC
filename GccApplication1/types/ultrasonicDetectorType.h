/*
 * ultrasonicDetectorType.h
 *
 * Created: 26-Mar-25 10:03:03 AM
 *  Author: kobac
 */ 


#ifndef ULTRASONICDETECTORTYPE_H_
#define ULTRASONICDETECTORTYPE_H_

#include "ultrasonic.h"
#include "ultrasonic_hal.h"
#include "bitmapType.h"
#include "TCRTType.h"

#define ULTRADET_ZONE_ULTRA_CLEAR BIT0_MASK
#define ULTRADET_ZONE_TRCT_U_DETECTING BIT1_MASK //Flag que marca que esta sensando
/*#define DISCARDED flags.bitmap.bit2;
#define DISCARDED flags.bitmap.bit3;*/
// NO AGREGAR MAS -> Nibble H lo usamos para el tipo

#define	ULTRADET_SENSOR_IDLE 0
#define	ULTRADET_SENSOR_DETECTING 1
#define	ULTRADET_SENSOR_WAITING_CLEAR 2

typedef struct{
	ultrasonic_t* sensor;
	TCRT_t* sensor_IR;
	Byte_Flag_Struct flags; //Nibble H para estado, Nibble L para flags
}Ultrasonic_Detector_t;

#endif /* ULTRASONICDETECTORTYPE_H_ */