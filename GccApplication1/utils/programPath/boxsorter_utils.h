/*
 * boxsorter_utils.h
 *
 * Created: 26-Mar-25 9:51:26 AM
 *  Author: kobac
 */ 


#ifndef BOXSORTER_UTILS_H_
#define BOXSORTER_UTILS_H_

#include "../../types/boxTypes.h"
//#include <stdbool.h>

void ultraSensorTask(Ultrasonic_Detector_t* ultraDetector, sorter_system_t * sorter);
void irSensorsTask(sorter_system_t * sorter);
void servosTask();
void protocolTask();
void initSorter(sorter_system_t* SystemSorter);
box_type_t classify_box(uint8_t height_mm, sorter_system_t* SystemSorter);

#endif /* BOXSORTER_UTILS_H_ */