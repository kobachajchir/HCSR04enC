/*
 * servo_utils.h
 *
 * Created: 23-Mar-25 9:41:46 PM
 *  Author: kobac
 */ 


#ifndef SERVO_UTILS_H_
#define SERVO_UTILS_H_

void initServo(volatile servo_t* servo, uint8_t index, uint8_t pin, uint8_t startAngle);
uint16_t calculate_angle_pulseUs(uint8_t angle);
void servo_set_angle(uint8_t index, uint8_t angle);
void pushServo(volatile servo_t* servo);

#endif /* SERVO_UTILS_H_ */