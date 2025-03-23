/*
 * CINTATYPE.h
 *
 * Created: 22-Mar-25 5:31:42 PM
 *  Author: kobac
 */ 


#ifndef CINTATYPE_H_
#define CINTATYPE_H_


typedef union {
	struct{
		uint8_t tipo;    // El 'tipo' será uno de los valores del enum CajaType
		uint16_t count;
		uint8_t nro_orden;
	}cinta_struct;
	uint32_t cinta_struct_full_mem;
} cinta_out;


#endif /* CINTATYPE_H_ */