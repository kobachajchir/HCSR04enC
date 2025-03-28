/*
 * servo_utils.h
 *
 * Created: 23-Mar-25 9:41:46 PM
 *  Author: kobac
 */ 


#ifndef SERVO_UTILS_H_
#define SERVO_UTILS_H_

void initServo(servo_t* servo, uint8_t index, uint8_t pin);
void servo_set_angle(uint8_t index, uint8_t angle);
void update_servos();


#endif /* SERVO_UTILS_H_ */