/*
 * boxsorter_utils.h
 *
 * Created: 26-Mar-25 9:51:26 AM
 *  Author: kobac
 */ 


#ifndef BOXSORTER_UTILS_H_
#define BOXSORTER_UTILS_H_

#include "../../types/boxTypes.h"

void ultraSensorTask(ultrasonic_t* us);
box_type_t classify_box(uint16_t height_mm, box_height_range_t *ranges);

#endif /* BOXSORTER_UTILS_H_ */