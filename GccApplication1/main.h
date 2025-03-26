/*
 * main.h
 *
 * Created: 22-Mar-25 5:47:24 PM
 *  Author: kobac
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include "types/bitmapType.h"
#include "types/ultrasonicDetectorType.h"
#include "types/sorterSystemTypes.h"
#include "ultrasonic.h"
#include "ultrasonic_hal.h"

#define NIBBLEH_SET_STATE(object, state) \
((object).flags.byte = ((object).flags.byte & 0x0F) | (((state) & 0x0F) << 4))

#define NIBBLEH_GET_STATE(object) (((object).flags.byte >> 4) & 0x0F)

#define ALL_FLAGS flags.byte

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
#define ECHO_INTERVAL_TENMS 10 //Max 2.5s que es 255, porque el contador es uint8

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
#define ECHO_INTERVAL_FLAG bandera.bitmap.bit3
#define SERVOA_MOVE bandera.bitmap.bit4
#define SERVOA_RESET bandera.bitmap.bit5
#define BTN_PRESSED bandera.bitmap.bit6 // Usamos el bit 6 para detectar el button press
#define ULTRASONIC_ENABLE bandera.bitmap.bit7 //

#define VEINTEMS_PASSED bandera2.bitmap.bit0
#define EMIT_TRIGGER bandera2.bitmap.bit1
#define EMIT_FAILED bandera2.bitmap.bit2
#define WAITING_ECHO bandera2.bitmap.bit3
#define WAIT_TIME_TRIGGER_PASSED bandera2.bitmap.bit4
#define TRIGGER_ACTIVE bandera2.bitmap.bit5
#define DEBUG_FLAGS bandera2.bitmap.bit6 //DebugFlags de la libreria del HCSR04
#define DEBUG_FLAGS_SORTER bandera2.bitmap.bit7 //DebugFlags del Sorter

extern Byte_Flag_Struct bandera;  // Definido para manejar flags //Se manejan dentro de las interrupciones, por eso son volatile estas
extern Byte_Flag_Struct bandera2;  // Definido para manejar flags //Se manejan dentro de las interrupciones, por eso son volatile estas
extern volatile uint16_t echo_init_time;  // Tiempo de inicio (flanco ascendente)
extern volatile uint16_t echo_finish_time;    // Tiempo final (flanco descendente)
extern volatile uint16_t distance_mm;      // Distancia en milímetros
extern volatile uint32_t pulse_width_A; // Starting position
extern volatile uint8_t ovf_count; // Contador de desbordamientos del Timer 1
extern volatile uint8_t wait_time; // Contador de desbordamientos del Timer 1
extern volatile uint8_t btn_pressed_time; // Contador de btn presionado en multiplos de 10ms
extern volatile uint8_t echo_state; // Estado de la señal de eco
extern volatile uint8_t servo_counter;
extern ultrasonic_t ultraSensor;
extern Ultrasonic_Detector_t hcsr04Detector;
extern sorter_system_t SorterSystem;

#endif /* MAIN_H_ */