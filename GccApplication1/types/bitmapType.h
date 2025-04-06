/*
 * BITMAPTYPE.h
 *
 * Created: 22-Mar-25 5:31:54 PM
 *  Author: kobac
 */ 


#ifndef BITMAPTYPE_H_
#define BITMAPTYPE_H_

#include <stdint.h> 

typedef union {
	struct {
		uint8_t bit0: 1;  // Bit 0 - Parte del nibble bajo (bitL)
		uint8_t bit1: 1;  // Bit 1 - Parte del nibble bajo (bitL)
		uint8_t bit2: 1;  // Bit 2 - Parte del nibble bajo (bitL)
		uint8_t bit3: 1;  // Bit 3 - Parte del nibble bajo (bitL)
		uint8_t bit4: 1;  // Bit 4 - Parte del nibble alto (bitH)
		uint8_t bit5: 1;  // Bit 5 - Parte del nibble alto (bitH)
		uint8_t bit6: 1;  // Bit 6 - Parte del nibble alto (bitH)
		uint8_t bit7: 1;  // Bit 7 - Parte del nibble alto (bitH)
	} bitmap;
	struct {
		uint8_t bitL: 4;  // Nibble bajo (bits 0-3)
		uint8_t bitH: 4;  // Nibble alto (bits 4-7)
	} nibbles;
	uint8_t byte;
} Byte_Flag_Struct;

#endif /* BITMAPTYPE_H_ */