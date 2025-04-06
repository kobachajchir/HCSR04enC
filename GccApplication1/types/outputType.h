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
#include "boxTypes.h"

#define OUTPUT_BUSY BIT0_MASK
#define OUTPUT_PUSH BIT1_MASK
#define OUTPUT_RESET BIT2_MASK
#define OUTPUT_READY BIT3_MASK
//Solo NIBBLE L para flags

#define OUTPUT_A_DEFAULT_BOX_TYPE BOX_SIZE_A
#define OUTPUT_B_DEFAULT_BOX_TYPE BOX_SIZE_B
#define OUTPUT_C_DEFAULT_BOX_TYPE BOX_SIZE_C

typedef struct {
	uint8_t sensor_pin;         // Pin digital del sensor de paso
	uint8_t actuator_pin;       // Pin digital del actuador
	Byte_Flag_Struct flags; //Nibble H para el box_type y Nibble L para las banderas
	box_type_t boxType;
} output_t;


#endif /* OUTPUTTYPE_H_ */