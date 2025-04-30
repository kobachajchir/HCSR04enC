/*
 * boxTypes.h
 *
 * Created: 26-Mar-25 9:22:39 AM
 *  Author: kobac
 */ 

#ifndef BOXTYPES_H_
#define BOXTYPES_H_

#include "bitmapType.h"
#include "../main.h"

// #define ZONE_ULTRA_CLEAR BIT0_MASK;
// #define SELECTED BIT1_MASK;
// #define DISCARDED BIT2_MASK;
// #define DISCARDED BIT3_MASK;
//NO AGREGAR MAS -> Nibble H lo usamos para el tipo 

//Definicion de constantes de dimensiones de las cajas
#define BOX_A_MIN_MM 55
#define BOX_A_MAX_MM 65
#define BOX_B_MIN_MM 75
#define BOX_B_MAX_MM 85
#define BOX_C_MIN_MM 95
#define BOX_C_MAX_MM 105
#define BOX_LENGTH_MM 80

//Definiciones de los tipos de cajas -> Nibble H de las flags
#define BOX_DISCARDED 0  // Define de descarte
#define BOX_SIZE_A 1  // Define de caja grande
#define BOX_SIZE_B 2  // Define de caja media
#define BOX_SIZE_C 3  // Define de caja chica
#define NO_BOX 4 //No definida
typedef uint8_t box_type_t; //Tipo de caja

//Nibble L de las flag
#define BOX_WAITING_MEASURE 0   // Esperando en zona de medición
#define BOX_MEASURED 1            // Altura ya medida y clasificada
#define BOX_MOVING 2    // En tránsito hacia una salida
#define BOX_AT_OUTPUT 3    // Detectada en sensor IR
#define BOX_PUSHED  4    // Ya empujada
#define BOX_COMPLETED 5 //Listo, guardar en estadisticas
typedef uint8_t box_state_t; //Estado de la caja

/// @brief Tamaño máximo de la cola (cálculo basado en una línea de producción de 2.1m y cajas de 80mm).
#define BOX_QUEUE_SIZE 26

/**
 * @struct box_queue_t
 * @brief Representa una cola circular para almacenar eventos de cajas.
 *
 * Cada evento puede incluir información de la caja (por ejemplo, la altura medida o el tipo).
 */
typedef struct {
	box_t events[BOX_QUEUE_SIZE]; /**< Array de eventos de caja */
	uint8_t head;                 /**< Índice de extracción (caja más antigua) */
	uint8_t tail;                 /**< Índice de inserción (posición para una nueva caja) */
} box_queue_t;

typedef struct {
	uint8_t min_height_mm;  // Minimum height in millimeters
	uint8_t max_height_mm;  // Maximum height in millimeters
} box_height_range_t;

typedef struct {
	box_height_range_t box_size_a;  // Large box range
	box_height_range_t box_size_b;  // Medium box range
	box_height_range_t box_size_c;  // Small box range
} box_height_range_matrix_t;

typedef struct {
	uint8_t height_mm;     // Altura medida
	Byte_Flag_Struct flags; //Nibble H tiene el tipo, Nibble L tiene las banderas
} box_t;


#endif /* BOXTYPES_H_ */