/**
 * @file sorterSystemTypes.h
 * @brief Tipos de datos relacionados con el sistema de clasificación de cajas.
 *
 * Define estructuras y constantes necesarias para gestionar la lógica del sistema 
 * de clasificación, incluyendo estadísticas de producción y salidas disponibles.
 *
 * @author kobac
 * @date 26-Mar-25
 */

#ifndef SORTERSYSTEMTYPES_H_
#define SORTERSYSTEMTYPES_H_

#include "boxTypes.h"
#include "outputType.h"

/// @brief Número total de salidas del sistema.
#define NUM_OUTPUTS 3
/// @brief Altura máxima permitida para la detección (en mm).
#define DETECTION_THRESHOLD_MAX_MM 105
/// @brief Altura mínima permitida para considerar una caja (en mm).
#define DETECTION_THRESHOLD_MIN_MM 55
/// @brief Distancia de referencia cuando no hay caja (en mm).
#define DETECTION_IDLE_DISTANCE_MM 190

/**
 * @struct production_stats_t
 * @brief Contiene estadísticas de producción del sistema clasificador.
 *
 * Incluye el total de cajas medidas, clasificadas por tipo, y la cantidad descartada.
 */
typedef struct {
	uint16_t total_measured;
	uint16_t total_by_type_array[3];
	uint16_t total_discarded;
} production_stats_t;

/**
 * @struct sorter_system_t
 * @brief Representa el sistema de clasificación completo.
 *
 * Incluye los rangos de altura válidos para cada tipo de caja, punteros a las salidas
 * (actuadores), y las estadísticas de producción actuales.
 */
typedef struct {
	box_height_range_matrix_t* box_ranges;
	output_t* outputs[NUM_OUTPUTS];
	production_stats_t stats;
} sorter_system_t;


#endif /* SORTERSYSTEMTYPES_H_ */