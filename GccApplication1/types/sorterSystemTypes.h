/*
 * sorterSystemTypes.h
 *
 * Created: 26-Mar-25 9:36:23 AM
 *  Author: kobac
 */ 


#ifndef SORTERSYSTEMTYPES_H_
#define SORTERSYSTEMTYPES_H_

#include "boxTypes.h"
#include "outputType.h"

#define NUM_OUTPUTS 3
#define DETECTION_THRESHOLD_MAX_MM 105
#define DETECTION_THRESHOLD_MIN_MM 55
#define DETECTION_IDLE_DISTANCE_MM 190

typedef struct {
	uint16_t total_measured;
	uint16_t total_by_type_array[3];
	uint16_t total_discarded;
} production_stats_t;

typedef struct {
	box_height_range_matrix_t* box_ranges;
	output_t* outputs[NUM_OUTPUTS];
	production_stats_t stats;
} sorter_system_t;


#endif /* SORTERSYSTEMTYPES_H_ */