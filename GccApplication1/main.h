/*
 * main.h
 *
 * Created: 22-Mar-25 5:47:24 PM
 *  Author: kobac
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include "utils/macros_utils.h"
#include "types/bitmapType.h"
#include "types/ultrasonicDetectorType.h"
#include "types/sorterSystemTypes.h"
#include "types/TCRTType.h"
#include "types/outputType.h"
#include "types/servoType.h"
#include "types/protocolTypes.h"
#include "ultrasonic.h"
#include "ultrasonic_hal.h"
#include <avr/io.h>

#define ALL_FLAGS flags.byte

#define F_CPU 16000000UL  // Definir la frecuencia del reloj en 16 MHz
#define TRIGGER_PIN  PD3  // Pin de Trigger PIN Numero 3
#define TRIGGER_DDR    DDRD
#define TRIGGER_PORT   PORTD
#define ECHO_PIN     PB0  // Pin de Echo PIN Numero 8
#define BUTTON_PIN     PD4  // Pin de Button PIN Numero 4
#define LED_BUILTIN_PIN PB5  // Pin LED incorporado (Arduino pin 13)
#define SERVOA_PIN PB1  // Pin Servo A PIN 9
#define SERVOB_PIN PB2  // Pin Servo B PIN 10
#define SERVOC_PIN PB3  // Pin Servo C PIN 11

#define TCRT_A PC0
#define TCRT_B PC1
#define TCRT_C PC2
#define TCRT_U PC3

#define TCRT_A_CHANNEL 0
#define TCRT_B_CHANNEL 1
#define TCRT_C_CHANNEL 2
#define TCRT_U_CHANNEL 3

#define TCRT_CALIBRATION_SAMPLES 10
#define TCRT_FILTER_SAMPLES 10

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

#define SERVO_PUSH_ANGLE 0
#define SERVO_IDLE_ANGLE 90

#define BTN_PRESS_TIME 10  // Define de descarte

#define PROTOCOL_MIN_BYTE_COUNT 6
#define PROTOCOL_MAX_BYTE_COUNT 24


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
#define IR_READ_INTERRUPT bandera.bitmap.bit4
#define IR_CALIBRATED bandera.bitmap.bit5
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

#define OUTPUT_A_HAS_CONFIG bandera3.bitmap.bit0
#define OUTPUT_B_HAS_CONFIG bandera3.bitmap.bit1
#define OUTPUT_C_HAS_CONFIG bandera3.bitmap.bit2
#define IS_TRANSMITTING bandera3.bitmap.bit3
#define CREATE_RESPONSE_PCK bandera3.bitmap.bit4

#define SERVO_ACTIVE_TIME 10

//Defines para bits individuales
#define BIT0_MASK   0x01  // 0000 0001
#define BIT1_MASK   0x02  // 0000 0010
#define BIT2_MASK   0x04  // 0000 0100
#define BIT3_MASK   0x08  // 0000 1000
#define BIT4_MASK   0x10  // 0001 0000
#define BIT5_MASK   0x20  // 0010 0000
#define BIT6_MASK   0x40  // 0100 0000
#define BIT7_MASK   0x80  // 1000 0000

//Defines para combinaciones de 2 bit
#define BITS01_MASK 0x03  // 0000 0011
#define BITS23_MASK 0x0C  // 0000 1100
#define BITS45_MASK 0x30  // 0011 0000
#define BITS67_MASK 0xC0  // 1100 0000

//Defines para nibbles
#define NIBBLE_L_MASK 0x0F  // 0000 1111 ? bits 0-3
#define NIBBLE_H_MASK 0xF0  // 1111 0000 ? bits 4-7

extern Byte_Flag_Struct bandera;  // Definido para manejar flags //Se manejan dentro de las interrupciones, por eso son volatile estas
extern Byte_Flag_Struct bandera2;  // Definido para manejar flags //Se manejan dentro de las interrupciones, por eso son volatile estas
extern Byte_Flag_Struct bandera3;  // Definido para manejar flags //Se manejan dentro de las interrupciones, por eso son volatile estas
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
extern TCRT_t IR_A;
extern TCRT_t IR_B;
extern TCRT_t IR_C;
extern TCRT_t IR_U;
extern servo_t servoA;
extern servo_t servoB;
extern servo_t servoC;
extern volatile servo_t* servosArray[NUM_OUTPUTS];
extern volatile uint8_t current_servo;
extern output_t salidaA;
extern output_t salidaB;
extern output_t salidaC;
extern ProtocolService protocolService;


#endif /* MAIN_H_ */