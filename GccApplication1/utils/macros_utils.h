/*
 * macros_utils.h
 *
 * Created: 27-Mar-25 12:08:38 PM
 *  Author: kobac
 */ 


#ifndef MACROS_UTILS_H_
#define MACROS_UTILS_H_

#include "../main.h"

#define SET_FLAG(flag_struct, BIT_MASK)     ((flag_struct).byte |=  (BIT_MASK)) //Sirven para todo tipo de combinaciones
#define CLEAR_FLAG(flag_struct, BIT_MASK)   ((flag_struct).byte &= ~(BIT_MASK)) //Pueden recibir valores de NIBBLES, BITSXY o BITX
#define TOGGLE_FLAG(flag_struct, BIT_MASK)  ((flag_struct).byte ^=  (BIT_MASK))	//Enviar la variable de flags
#define IS_FLAG_SET(flag_struct, BIT_MASK)  (((flag_struct).byte & (BIT_MASK)) != 0)//Usar con el tipo Byte_Flag_Struct;

// Setea nibble bajo (bits 0–3) con el valor deseado
#define NIBBLEL_SET_STATE(object, state) \
((object).byte = ((object).byte & NIBBLE_H_MASK) | ((state) & NIBBLE_L_MASK))

// Setea nibble alto (bits 4–7) con el valor deseado
#define NIBBLEH_SET_STATE(object, state) \
((object).byte = ((object).byte & NIBBLE_L_MASK) | (((state) & NIBBLE_L_MASK) << 4))


#define NIBBLEH_GET_STATE(flag_struct) (((flag_struct).byte & NIBBLE_H_MASK) >> 4)
#define NIBBLEL_GET_STATE(flag_struct) ((flag_struct).byte & NIBBLE_L_MASK)

#endif /* MACROS_UTILS_H_ */