/*
 * main.h
 *
 * Created: 22-Mar-25 5:47:24 PM
 *  Author: kobac
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include "types/bitmapType.h"
#include "types/cintaType.h"
#include "ultrasonic.h"
#include "ultrasonic_hal.h"

#define CAJA_GRANDE 3  // Define de caja grande
#define CAJA_MEDIA 2  // Define de caja media
#define CAJA_CHICA 1  // Define de caja chica
#define DESCARTE 0  // Define de descarte

#define F_CPU 16000000UL  // Definir la frecuencia del reloj en 16 MHz
#define TRIGGER_PIN  PD3  // Pin de Trigger PIN Numero 3
#define TRIGGER_DDR    DDRD
#define TRIGGER_PORT   PORTD
#define ECHO_PIN     PB0  // Pin de Echo PIN Numero 8
#define BUTTON_PIN     PD4  // Pin de Button PIN Numero 4
#define LED_BUILTIN_PIN PB5  // Pin LED incorporado (Arduino pin 13)
#define SERVOA_PIN PB1  // Pin Servo A PIN 6

#define DISTANCE_MIN_MM 30 //3cm min
#define DISTANCE_MAX_MM 2000 //2m max
#define ECHO_INTERVAL_TENMS 20 //100 es 1seg

#define SERVO_MIN_PULSE 2000UL // 1ms (2000 * 0.5us)
#define SERVO_START_PULSE 3000UL // 1ms (2000 * 0.5us)
#define SERVO_MAX_PULSE 4000UL // 2ms (4000 * 0.5us)
#define SERVO_FRAME_PERIOD 40000 // 20ms (40000 * 0.5us) o 50hz
#define SERVO_RESET_TIME 30 //Multiplos de 10ms

#define CAJA_GRANDE 3  // Define de caja grande
#define CAJA_MEDIA 2  // Define de caja media
#define CAJA_CHICA 1  // Define de caja chica
#define DESCARTE 0  // Define de descarte

#define BTN_PRESS_TIME 10  // Define de descarte

//#define DO_TRIGGER bandera.bitmap.bit0
//#define TRIGGER_FINISH bandera.bitmap.bit1
//#define ECHO_RISING bandera.bitmap.bit2
//#define ECHO_STATE bandera.bitmap.bit3  // Usamos el bit 3 para el estado de ECHO
// #define TRIGGER_STATE bandera.bitmap.bit4 // Usamos el bit 4 para el estado de TRIGGER
// #define TRIGGER_ALLOWED bandera.bitmap.bit5 // Usamos el bit 5 para el allow del TRIGGER
// #define CALCULATE bandera.bitmap.bit6 // Usamos el bit 6 para el allow del TRIGGER

#define BTN_RELEASED bandera.bitmap.bit0
#define TIMER2_ACTIVE bandera.bitmap.bit1
#define BTN_OVF bandera.bitmap.bit2
#define SECPASSED bandera.bitmap.bit3
#define SERVOA_MOVE bandera.bitmap.bit4
#define SERVOA_RESET bandera.bitmap.bit5
#define BTN_PRESSED bandera.bitmap.bit6 // Usamos el bit 6 para detectar el button press
#define ULTRASONIC_ENABLE bandera.bitmap.bit7 //

#define VEINTEMS_PASSED bandera2.bitmap.bit0

/*
#define ECHO_STATE bandera.bitmap.bit3  // Usamos el bit 3 para el estado de ECHO
#define TRIGGER_STATE bandera.bitmap.bit4 // Usamos el bit 4 para el estado de TRIGGER
#define TRIGGER_ALLOWED bandera.bitmap.bit5 // Usamos el bit 5 para el allow del TRIGGER
#define CALCULATE bandera.bitmap.bit6 // Usamos el bit 6 para el allow del TRIGGER
#define BTN_PRESS bandera.bitmap.bit7 // Usamos el bit 7 para detectar el button press
*/

extern Byte_Flag_Struct bandera;  // Definido para manejar flags
extern Byte_Flag_Struct bandera2;  // Definido para manejar flags
extern volatile uint16_t echo_init_time;  // Tiempo de inicio (flanco ascendente)
extern volatile uint16_t echo_finish_time;    // Tiempo final (flanco descendente)
extern volatile uint16_t distance_mm;      // Distancia en milímetros
extern volatile uint32_t pulse_width_A; // Starting position
extern volatile uint8_t ovf_count; // Contador de desbordamientos del Timer 1
extern volatile uint8_t wait_time; // Contador de desbordamientos del Timer 1
extern volatile uint8_t btn_pressed_time; // Contador de btn presionado en multiplos de 10ms
extern volatile uint8_t echo_state; // Estado de la señal de eco
extern volatile uint8_t servo_counter;
extern cinta_out outA;
extern cinta_out outB;
extern cinta_out outC;
extern cinta_out outD;
extern ultrasonic_t ultraSensor;
#endif /* MAIN_H_ */