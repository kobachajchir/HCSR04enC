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

#define ZONE_ULTRA_CLEAR flags.bitmap.bit0;
/*#define SELECTED flags.bitmap.bit1;
#define DISCARDED flags.bitmap.bit2;
#define DISCARDED flags.bitmap.bit3;*/
// NO AGREGAR MAS -> Nibble H lo usamos para el tipo

#define	SENSOR_IDLE 0
#define	SENSOR_DETECTING 1
#define	SENSOR_WAITING_CLEAR 2
typedef uint8_t detection_state_t;

typedef struct{
	ultrasonic_t* sensor;
	Byte_Flag_Struct flags; //Nibble H para estado, Nibble L para flags
}Ultrasonic_Detector_t;

#endif /* ULTRASONICDETECTORTYPE_H_ */