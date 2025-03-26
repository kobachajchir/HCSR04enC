/*
 * outputType.h
 *
 * Created: 26-Mar-25 9:34:52 AM
 *  Author: kobac
 */ 


#ifndef OUTPUTTYPE_H_
#define OUTPUTTYPE_H_

#include "boxTypes.h"
#include "bitmapType.h"

#define OUTPUT_BUSY flags.bitmap.bit0;

typedef struct {
	uint8_t sensor_pin;         // Pin digital del sensor de paso
	uint8_t actuator_pin;       // Pin digital del actuador
	Byte_Flag_Struct flags; //Nibble H para el box_type y Nibble L para las banderas
} output_t;


#endif /* OUTPUTTYPE_H_ */