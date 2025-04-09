/*
 * main.h
 *
 * Created: 22-Mar-25 5:47:24 PM
 * Author: kobac
 */

#ifndef MAIN_H_
#define MAIN_H_

// ====================== INCLUDES ========================= //
#include "utils/macros_utils.h"
#include "types/bitmapType.h"
#include "types/ultrasonicDetectorType.h"
#include "types/sorterSystemTypes.h"
#include "types/TCRTType.h"
#include "types/outputType.h"
#include "types/servoType.h"
#include "types/protocolTypes.h"
#include "types/configType.h"
#include "ultrasonic.h"
#include "ultrasonic_hal.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

// ====================== DEFINES ========================= //

// --- Frecuencia de reloj ---
#define F_CPU 16000000UL  // 16 MHz

// --- Pines del sistema ---
#define TRIGGER_PIN      PD3 //PIN 3
#define TRIGGER_DDR      DDRD
#define TRIGGER_PORT     PORTD
#define ECHO_PIN         PB0 //PIN 8
#define BUTTON_PIN       PD4 //PIN 4
#define LED_BUILTIN_PIN  PB5 //PIN 13
#define SERVOA_PIN       PB1 //PIN 9
#define SERVOB_PIN       PB2 //PIN 10
#define SERVOC_PIN       PB3 //PIN 11

// 	// --- Pines TCRT ---
// 	#define TCRT_A PC0
// 	#define TCRT_B PC1
// 	#define TCRT_C PC2
// 	#define TCRT_U PC3
// 
// 	#define TCRT_A_CHANNEL 0
// 	#define TCRT_B_CHANNEL 1
// 	#define TCRT_C_CHANNEL 2
// 	#define TCRT_U_CHANNEL 3

#define TCRT_A PD5 //PIN 5
#define TCRT_B PD6 //PIN 6
#define TCRT_C PD7 //PIN 7
#define TCRT_U PD2 //PIN 2

// --- Parámetros TCRT ---
#define TCRT_CALIBRATION_SAMPLES 10
#define TCRT_FILTER_SAMPLES 10

// --- Parámetros de distancia ultrasónica ---
#define DISTANCE_MIN_MM 30
#define DISTANCE_MAX_MM 2000
#define ECHO_INTERVAL_TENMS 10
#define TRIGGER_WAIT_TIME_TENMS 6

// --- Parámetros Servo ---
#define SERVO_MIN_PULSE     2000UL
#define SERVO_START_PULSE   3000UL
#define SERVO_MAX_PULSE     4000UL
#define SERVO_FRAME_PERIOD  40000
#define SERVO_RESET_TIME    30
#define SERVO_ACTIVE_TIME   10
#define SERVO_PUSH_ANGLE    0
#define SERVO_IDLE_ANGLE    90

// --- Tipos de caja (para mapeo lógico) ---
#define CAJA_GRANDE 3
#define CAJA_MEDIA  2
#define CAJA_CHICA  1
#define DESCARTE    0

// --- Parámetros de botón ---
#define BTN_PRESS_TIME 10

// --- Parámetros de protocolo ---
#define PROTOCOL_MIN_BYTE_COUNT 6
#define PROTOCOL_MAX_BYTE_COUNT 24
#define PROTOCOL_FIXED_SIZE     8

// --- Información del dispositivo ---
#define DEV_REPOSITORY        "github.com/kobachajchir/HCSR04enC"
#define DEV_FIRMWARE_VERSION "1.0.0"

// ====================== FLAGS ========================= //

// --- Byte de flags generales ---
#define ALL_FLAGS flags.byte

// --- Byte 1 ---
#define BTN_RELEASED            bandera.bitmap.bit0
#define TIMER2_ACTIVE           bandera.bitmap.bit1
#define BTN_OVF                bandera.bitmap.bit2
#define ECHO_INTERVAL_FLAG     bandera.bitmap.bit3
#define IR_READ_INTERRUPT      bandera.bitmap.bit4
#define IR_CALIBRATED          bandera.bitmap.bit5
#define BTN_PRESSED            bandera.bitmap.bit6
#define ULTRASONIC_ENABLE      bandera.bitmap.bit7

// --- Byte 2 ---
#define VEINTEMS_PASSED            bandera2.bitmap.bit0
#define EMIT_TRIGGER               bandera2.bitmap.bit1
#define EMIT_FAILED                bandera2.bitmap.bit2
#define WAITING_ECHO               bandera2.bitmap.bit3
#define WAIT_TIME_TRIGGER_PASSED  bandera2.bitmap.bit4
#define TRIGGER_ACTIVE             bandera2.bitmap.bit5
#define DEBUG_FLAGS                bandera2.bitmap.bit6
#define DEBUG_FLAGS_SORTER         bandera2.bitmap.bit7

// --- Byte 3 ---
#define DEBUG_FLAGS_SERVOS       bandera3.bitmap.bit0
#define DEBUG_FLAGS_EEPROM       bandera3.bitmap.bit1
#define IS_TRANSMITTING          bandera3.bitmap.bit3
#define CREATE_RESPONSE_PCK      bandera3.bitmap.bit4
#define DO_ACTION                bandera3.bitmap.bit5
#define NEW_CONFIG               bandera3.bitmap.bit6

// ====================== Máscaras ========================= //

// --- Bits individuales ---
#define BIT0_MASK 0x01
#define BIT1_MASK 0x02
#define BIT2_MASK 0x04
#define BIT3_MASK 0x08
#define BIT4_MASK 0x10
#define BIT5_MASK 0x20
#define BIT6_MASK 0x40
#define BIT7_MASK 0x80

// --- Pares de bits ---
#define BITS01_MASK 0x03
#define BITS23_MASK 0x0C
#define BITS45_MASK 0x30
#define BITS67_MASK 0xC0

// --- Nibbles ---
#define NIBBLE_L_MASK 0x0F
#define NIBBLE_H_MASK 0xF0

// ====================== VARIABLES EXTERNAS ========================= //

extern Byte_Flag_Struct bandera;
extern Byte_Flag_Struct bandera2;
extern Byte_Flag_Struct bandera3;

extern volatile uint16_t echo_init_time;
extern volatile uint16_t echo_finish_time;
extern volatile uint16_t distance_mm;
extern volatile uint32_t pulse_width_A;
extern volatile uint8_t ovf_count;
extern volatile uint8_t wait_time;
extern volatile uint8_t btn_pressed_time;
extern volatile uint8_t echo_state;
extern volatile uint8_t servo_counter;
extern volatile uint8_t transmit_counter;
extern uint8_t doActionCmd;

// --- Objetos del sistema ---
extern ultrasonic_t ultraSensor;
extern Ultrasonic_Detector_t hcsr04Detector;
extern sorter_system_t SorterSystem;

// --- Sensores TCRT ---
extern TCRT_t IR_A;
extern TCRT_t IR_B;
extern TCRT_t IR_C;
extern TCRT_t IR_U;

// --- Servos ---
extern servo_t servoA;
extern servo_t servoB;
extern servo_t servoC;
extern volatile servo_t* servosArray[NUM_OUTPUTS];
extern volatile uint8_t current_servo;

// --- Salidas ---
extern output_t salidaA;
extern output_t salidaB;
extern output_t salidaC;

// --- Protocolo de comunicación ---
extern ProtocolService protocolService;

// --- Configuración EEPROM ---
extern Config_t currentConfig;
extern EEMEM Config_t eepromConfig;

#endif /* MAIN_H_ */
