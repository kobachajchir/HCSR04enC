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
#define DETECTION_IDLE_DISTANCE_MM 150

typedef struct {
	uint32_t total_measured;
	uint32_t total_selected;
	uint32_t total_discarded;
	uint32_t count_by_type[BOX_DISCARDED + 1]; // Indexado por box_type_t
} production_stats_t;

typedef struct {
	box_height_range_matrix_t *box_ranges;
	output_t outputs[NUM_OUTPUTS];
	production_stats_t stats;
} sorter_system_t;


#endif /* SORTERSYSTEMTYPES_H_ */