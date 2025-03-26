/*
 * BITMAPTYPE.h
 *
 * Created: 22-Mar-25 5:31:54 PM
 *  Author: kobac
 */ 


#ifndef BITMAPTYPE_H_
#define BITMAPTYPE_H_

typedef union{
	struct{
		uint8_t bit0: 1;
		uint8_t bit1: 1;
		uint8_t bit2: 1;
		uint8_t bit3: 1;
		uint8_t bit4: 1;
		uint8_t bit5: 1;
		uint8_t bit6: 1;
		uint8_t bit7: 1;
	}bitmap;
	uint8_t byte;
}Byte_Flag_Struct;

#endif /* BITMAPTYPE_H_ */