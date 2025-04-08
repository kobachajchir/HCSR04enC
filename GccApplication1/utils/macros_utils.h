/*
 * macros_utils.h
 *
 * Created: 27-Mar-25 12:08:38 PM
 * Author: kobac
 */

#ifndef MACROS_UTILS_H_
#define MACROS_UTILS_H_

#include "../types/bitmapType.h"  // Importa definiciones globales, máscaras y estructuras

// ===================== FLAGS ============================== //
// --- Macros generales para manipular flags de tipo Byte_Flag_Struct ---

// Setea los bits especificados en el byte de flags
#define SET_FLAG(flag_struct, BIT_MASK)     ((flag_struct).byte |=  (BIT_MASK))

// Limpia los bits especificados
#define CLEAR_FLAG(flag_struct, BIT_MASK)   ((flag_struct).byte &= ~(BIT_MASK))

// Cambia el estado (toggle) de los bits especificados
#define TOGGLE_FLAG(flag_struct, BIT_MASK)  ((flag_struct).byte ^=  (BIT_MASK))

// Verifica si uno o más bits están seteados (devuelve true si alguno está activo)
#define IS_FLAG_SET(flag_struct, BIT_MASK)  (((flag_struct).byte & (BIT_MASK)) != 0)

// ===================== NIBBLES ============================== //
// --- Operaciones específicas para nibbles (mitades de un byte) ---
// Esto permite codificar estados dentro de 4 bits, dividiendo el byte en 2.

// Setea el nibble bajo (bits 0–3) al valor dado, preservando el alto
#define NIBBLEL_SET_STATE(object, state) \
    ((object).byte = ((object).byte & NIBBLE_H_MASK) | ((state) & NIBBLE_L_MASK))

// Setea el nibble alto (bits 4–7) al valor dado, preservando el bajo
#define NIBBLEH_SET_STATE(object, state) \
    ((object).byte = ((object).byte & NIBBLE_L_MASK) | (((state) & NIBBLE_L_MASK) << 4))

// Obtiene el estado actual del nibble alto (4 bits de estado)
#define NIBBLEH_GET_STATE(flag_struct) (((flag_struct).byte & NIBBLE_H_MASK) >> 4)

// Obtiene el estado actual del nibble bajo
#define NIBBLEL_GET_STATE(flag_struct) ((flag_struct).byte & NIBBLE_L_MASK)

#endif /* MACROS_UTILS_H_ */
