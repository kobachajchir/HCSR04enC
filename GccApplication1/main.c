/* Includes ------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "main.h"
#include "utils/macros_utils.h"
#include "types/bitmapType.h"
#include "types/sorterSystemTypes.h"
#include "utils/usart/usart_utils.h"
#include "utils/servo/servo_utils.h"
#include "types/ultrasonicDetectorType.h"
#include "types/TCRTType.h"
#include "types/configType.h"
#include "utils/programPath/boxsorter_utils.h"
#include "utils/protocol/protocol_utils.h"
#include "ultrasonic.h"
#include "ultrasonic_hal.h"

/* END Includes --------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
Byte_Flag_Struct bandera;  // Definido para manejar flags
Byte_Flag_Struct bandera2;  // Definido para manejar flags
Byte_Flag_Struct bandera3;  // Definido para manejar flags
volatile uint16_t echo_init_time = 0;  // Tiempo de inicio (flanco ascendente)
volatile uint16_t echo_finish_time = 0;    // Tiempo final (flanco descendente)
volatile uint16_t distance_mm = 0;      // Distancia en milímetros
volatile uint8_t servo_counter = 0;      // servo tick counter
volatile uint32_t pulse_width_A = SERVO_START_PULSE; // Starting position
volatile uint16_t target_tick_A = 0; //Target de ticks para modificar estado softPWM ServoA
volatile uint8_t ovf_count = 0; // Contador de desbordamientos del Timer 1
volatile uint8_t wait_time = 0; // Contador de desbordamientos del Timer 1
volatile uint8_t btn_pressed_time = 0; // Contador de btn presionado en multiplos de 10ms
volatile uint8_t echo_state = 0; // Estado de la señal de eco
volatile uint8_t diezMsCounter = 0; //Counter de 10ms, max 255 ovf cada 2.55 segundos
volatile uint8_t veintems_counter = 0;
volatile uint8_t trigger_timeout_counter = 0;
volatile uint8_t servo_active_counter = 0;
volatile uint8_t transmit_counter;
uint8_t doActionCmd;
uint8_t servoAVal = 0; //Angulo 0 a 180 Servo A
ultrasonic_t ultraSensor;
Ultrasonic_Detector_t hcsr04Detector;
TCRT_t IR_A;
TCRT_t IR_B;
TCRT_t IR_C;
TCRT_t IR_U;
servo_t servoA;
servo_t servoB;
servo_t servoC;
volatile servo_t* servosArray[NUM_OUTPUTS];
volatile uint8_t current_servo = 0;
output_t salidaA;
output_t salidaB;
output_t salidaC;
sorter_system_t SorterSystem;
ProtocolService protocolService;
Config_t currentConfig;
EEMEM Config_t eepromConfig;

/* END Global variables ------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void printfWrapper(const char* message);
void timer1_init(void);
void timer2_init(void);
static inline void gpio_pins_init(void);
static inline void buttonTask(void);
static inline bool calibrateAllIRSensors(void);

/* END Function prototypes ---------------------------------------------------*/


/* Function ISR --------------------------------------------------------------*/
/**
 * @brief ISR del evento de captura del Timer1 para el sensor ultrasónico.
 *
 * Esta rutina de servicio de interrupción se invoca cuando se produce un evento de captura (TIMER1_CAPT_vect).  
 * Según el estado del sensor ultrasónico (almacenado en la variable global `ultraSensor`), la ISR realiza:
 * - **ULTRA_WAIT_RISING:**  
 *   Si la bandera `ECHO_RISING` está activa, se guarda el tiempo inicial de eco en `echo_init_time` usando el valor de ICR1 
 *   sumado al número de desbordamientos (`ovf_count * 65536UL`), y se limpia la bandera.
 * - **ULTRA_WAIT_FALLING:**  
 *   Se guarda el tiempo final de eco en `echo_finish_time` (calculado de forma similar) y se activa la bandera `ECHO_RECEIVED`
 *   para indicar que se recibió la señal completa.
 *
 * @note Se utilizan variables globales como `ultraSensor` y `ovf_count`.
 *
 * @retVal No aplica (ISR).
 */
ISR(TIMER1_CAPT_vect)
{
	// Verificamos el estado actual del sensor
	switch (ultraSensor.state)
	{
		case ULTRA_WAIT_RISING:
		{
			if(ultraSensor.ECHO_RISING){
				ultraSensor.echo_init_time = (uint32_t)(ICR1 + (ovf_count * 65536UL)); //Lleno init time
				ultraSensor.ECHO_RISING = 0; //Bajo la bandera para marcar que llene init time
			}
			break;
		}
		case ULTRA_WAIT_FALLING:
		{
			ultraSensor.echo_finish_time = (uint32_t)(ICR1 + (ovf_count * 65536UL)); //Lleno finish time
			ultraSensor.ECHO_RECEIVED = 1; //Esto lo pone en modo de calcular
			break;
		}
	}
}

/**
 * @brief ISR de desbordamiento del Timer1.
 *
 * Esta rutina de interrupción se ejecuta cada vez que el Timer1 se desborda (TIMER1_OVF_vect).  
 * Incrementa el contador global `ovf_count`, que se utiliza junto con `ICR1` para calcular 
 * el tiempo total de captura en aplicaciones como medición con sensor ultrasónico.
 *
 * @note No recibe parámetros. Utiliza la variable global `ovf_count`.
 *
 * @retVal No aplica (ISR).
 */
ISR(TIMER1_OVF_vect)
{
	ovf_count++;  // Incrementa el contador de desbordamientos del Timer 1
}

/**
 * @brief ISR del Timer1 Compare Match A.
 *
 * Programa la siguiente interrupción cada 20 ms para manejar el ciclo completo de actualización
 * de servos. Recorre todos los servos y ajusta el pulso de control según su estado. Si el servo
 * está en modo "push", se le asigna un pulso máximo. Si no, se calcula el pulso a partir del ángulo.
 *
 * @note Marca `IR_READ_INTERRUPT = 1` para indicar que debe procesarse lectura IR.
 *       Solo activa el pin del primer servo en la lista, programando su apagado mediante OCR1B.
 *
 * @retVal No aplica (ISR).
 *
 * @see calculate_angle_pulseUs
 */
ISR(TIMER1_COMPA_vect) {
	OCR1A += SERVO_FRAME_PERIOD;
	IR_READ_INTERRUPT = 1;
	
	for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
		if (IS_FLAG_SET(servosArray[i]->flags, SERVO_ENABLE)) {
			if (IS_FLAG_SET(servosArray[i]->flags, SERVO_PUSH) &&
			!IS_FLAG_SET(servosArray[i]->flags, SERVO_RESET)) {
				servosArray[i]->pulse_us = SERVO_MAX_PULSE;
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("ISR COMPA PUSH %d...\n"), servosArray[i]->pin);	
				}
			} else {
				servosArray[i]->pulse_us = calculate_angle_pulseUs(servosArray[i]->angle);
			}
			if (i == 0) {
				PORTB |= (1 << servosArray[i]->pin);
				OCR1B = TCNT1 + servosArray[i]->pulse_us;
				current_servo = 0; 
			}
		}
	}
}

/**
 * @brief ISR del Timer1 Compare Match B.
 *
 * Se encarga de apagar el pin del servo actualmente activo y actualizar su estado si estaba
 * en modo de "push" con "reset". Luego avanza al siguiente servo activo y configura su señal
 * PWM correspondiente en el mismo frame de 20 ms.
 *
 * @note Al volver de un push, se limpia el estado y se posiciona el servo en el ángulo idle.
 *       También se liberan las flags OUTPUT_BUSY y se activa OUTPUT_READY en la salida correspondiente.
 *
 * @retVal No aplica (ISR).
 *
 * @see calculate_angle_pulseUs
 */
ISR(TIMER1_COMPB_vect) {
	PORTB &= ~(1 << servosArray[current_servo]->pin);
	if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_PUSH) &&
	IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_RESET)) {
		CLEAR_FLAG(servosArray[current_servo]->flags, SERVO_RESET);
		CLEAR_FLAG(servosArray[current_servo]->flags, SERVO_PUSH);
		servosArray[current_servo]->angle = SERVO_IDLE_ANGLE;
		servosArray[current_servo]->pulse_us = calculate_angle_pulseUs(servosArray[current_servo]->angle);
		if(current_servo == 0){
			SET_FLAG(salidaA.flags, OUTPUT_READY);
			CLEAR_FLAG(salidaA.flags, OUTPUT_BUSY);
			if(DEBUG_FLAGS_SERVOS){
				printf_P(PSTR("Volvio a posicion IDLE A\n"));
			}
		}else if(current_servo == 1){
			SET_FLAG(salidaB.flags, OUTPUT_READY);
			CLEAR_FLAG(salidaB.flags, OUTPUT_BUSY);
			if(DEBUG_FLAGS_SERVOS){
				printf_P(PSTR("Volvio a posicion IDLE B\n"));
			}
		}else if(current_servo == 2){
			SET_FLAG(salidaC.flags, OUTPUT_READY);
			CLEAR_FLAG(salidaC.flags, OUTPUT_BUSY);
			if(DEBUG_FLAGS_SERVOS){
				printf_P(PSTR("Volvio a posicion IDLE C\n"));
			}
		}
	}
	current_servo++;
	if (current_servo < NUM_OUTPUTS) { //Cuando sale ya proceso todos los servos
		if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_ENABLE)) {
			PORTB |= (1 << servosArray[current_servo]->pin);
			OCR1B = TCNT1 + servosArray[current_servo]->pulse_us;
			} else {
			PORTB &= ~(1 << servosArray[current_servo]->pin);
			current_servo++;
			if (current_servo < NUM_OUTPUTS && IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_ENABLE)) {
				PORTB |= (1 << servosArray[current_servo]->pin);
				OCR1B = TCNT1 + servosArray[current_servo]->pulse_us;
			}
		}
	}
}

/**
 * @brief ISR de recepción por USART.
 *
 * Interrupción que se activa al recibir un byte por USART. Guarda el byte en un buffer circular.
 * Si el buffer está lleno, sobrescribe el dato más antiguo. Además, verifica si hay suficiente
 * información para iniciar el procesamiento de un paquete.
 *
 * @note Esta rutina actualiza `indexW`, y avanza `indexR` si el buffer está lleno para evitar bloqueo.
 *
 * @retVal No aplica (ISR).
 */
ISR(USART_RX_vect)
{
	uint8_t received_byte = UDR0; 
	uint8_t next_indexW = (protocolService.indexW + 1) % PROTOCOL_BUFFER_SIZE;
	if (next_indexW == protocolService.indexR) {
		protocolService.indexR = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	}
	protocolService.buffer[protocolService.indexW] = received_byte;
	protocolService.indexW = next_indexW;
	uint8_t available;
	if (protocolService.indexW >= protocolService.indexR) {
		available = protocolService.indexW - protocolService.indexR;
		} else {
		available = PROTOCOL_BUFFER_SIZE - protocolService.indexR + protocolService.indexW;
	}
    if (available >= PROTOCOL_MIN_BYTE_COUNT && !IS_FLAG_SET(protocolService.flags, PROTOSERV_CHECKDATA)) {
	    SET_FLAG(protocolService.flags, PROTOSERV_CHECKDATA);
    }
}

/**
 * @brief ISR de transmisión por USART (UDRE).
 *
 * Envia secuencialmente los bytes del paquete almacenado en el buffer circular. Cuando se transmiten
 * todos los bytes del paquete (payload + (header + length + token + cmd + checksum)), desactiva la
 * interrupción de transmisión y reactiva la de recepción.
 *
 * @note Se utiliza un contador (`transmit_counter`) para saber cuántos bytes se han enviado.
 *
 * @retVal No aplica (ISR).
 */
ISR(USART_UDRE_vect) {//							UNER + : + len + cmd + cks = 8 
	if (transmit_counter < (protocolService.receivePck.length + PROTOCOL_FIXED_SIZE)) {
		transmit_counter++;
		UDR0 = protocolService.buffer[protocolService.indexR];
		protocolService.indexR = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	} else {
		NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_SEND_DONE);
		UCSR0B &= ~(1 << UDRIE0);
		IS_TRANSMITTING = 0;
		UCSR0B |= (1 << RXCIE0);
	}
}

/**
 * @brief ISR de comparación A del Timer 2 (TIMER2_COMPA_vect)
 *
 * Rutina de interrupción que se ejecuta cada 10 ms. Controla el proceso completo del sensor ultrasónico:
 * el pulso de trigger, el tiempo de espera por el echo, el restablecimiento del trigger,
 * y maneja el tiempo de activación de los servos, así como la gestión de flags de los sensores TCRT.
 *
 * @details
 * - Activa/desactiva el pulso de trigger del sensor ultrasónico.
 * - Maneja los timeout de echo y tiempo de espera entre triggers de 60ms.
 * - Actualiza flags para la lectura de sensores IR (TCRT).
 * - Gestiona el tiempo de activación (pulsos) de los servos y sus flags.
 * - Maneja la lectura del botón y sus estados de desbordamiento.
 *
 * @note Esta rutina se ejecuta solo si `TIMER2_ACTIVE` está activado.
 *
 * @retval No aplica (ISR)
 */

ISR(TIMER2_COMPA_vect)
{
	if(TIMER2_ACTIVE) {
		if(ultraSensor.state == ULTRA_TRIGGERING) {
			if(!TRIGGER_ACTIVE) {
				ultrasonic_hal_trigger_setHigh();
				ultraSensor.DO_TRIGGER = 1;
				TRIGGER_ACTIVE = 1;
				} else {
				ultrasonic_hal_trigger_setLow();
				ultraSensor.TRIGGER_FINISH = 1;
				ultraSensor.TRIGGER_ALLOWED = 0;
				ultraSensor.state = ULTRA_WAIT_RISING;
				ultraSensor.ECHO_RISING = 1; 
				ultraSensor.TRIGGER_FINISH = 0; 
				TRIGGER_ACTIVE = 0;
				EMIT_TRIGGER = 0;
				WAITING_ECHO = 1;
			}
		}
		
		// --- Timeout de 20ms para el ECHO ---
		if(ultraSensor.state == ULTRA_WAIT_RISING && ultraSensor.ECHO_RISING && !VEINTEMS_PASSED && WAITING_ECHO) {
			if(veintems_counter < 1) { // 1 * 10ms = 10ms (ajusta si lo deseas a 2 para 20ms)
				veintems_counter++;
				} else {
				VEINTEMS_PASSED = 1;
				veintems_counter = 0;
			}
		}
		// --- Habilitar el trigger nuevamente en estados IDLE o DONE (70ms) ---
		if(!ultraSensor.TRIGGER_ALLOWED && !WAIT_TIME_TRIGGER_PASSED) {
			if(wait_time < TRIGGER_WAIT_TIME_TENMS){ // 6 * 10ms = 60ms
				wait_time++;
				} else {
				WAIT_TIME_TRIGGER_PASSED = 1;
				wait_time = 0;
			}
			if(ultraSensor.TIMEDOUT){
				ultrasonic_timeout_clear(&ultraSensor, DEBUG_FLAGS ? true : false);
			}
		}
		//De esto depende cada cuanto se hace un trigger
		if(diezMsCounter < ECHO_INTERVAL_TENMS){
			diezMsCounter++;
			} else {
			ECHO_INTERVAL_FLAG = 1;
			diezMsCounter = 0;
		}
		
		if(BTN_PRESSED){
			if(btn_pressed_time == 255){
				BTN_OVF = 1;
			}
			btn_pressed_time++;
		}
		if(IS_FLAG_SET(servoA.flags, SERVO_PUSH)){
			if(servoA.state_time < SERVO_ACTIVE_TIME){
				servoA.state_time++;
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("Servo A PUSH: state_time = %d\n"), servoA.state_time);
				}
				} else {
				servoA.state_time = 0;
				SET_FLAG(servoA.flags, SERVO_RESET);
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("Servo A RESET"));
				}
			}
		}
		if(IS_FLAG_SET(servoB.flags, SERVO_PUSH)){
			if(servoB.state_time < SERVO_ACTIVE_TIME){
				servoB.state_time++; 
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("Servo B PUSH: state_time = %d\n"), servoA.state_time);
				}
				} else {
				servoB.state_time = 0;
				SET_FLAG(servoB.flags, SERVO_RESET);
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("Servo B RESET"));
				}
			}
		}
		if(IS_FLAG_SET(servoC.flags, SERVO_PUSH)){
			if(servoC.state_time < SERVO_ACTIVE_TIME){
				servoC.state_time++;
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("Servo C PUSH: state_time = %d\n"), servoA.state_time);
				}
				} else {
				servoC.state_time = 0;
				SET_FLAG(servoC.flags, SERVO_RESET);
				if(DEBUG_FLAGS_SERVOS){
					printf_P(PSTR("Servo C RESET"));
				}
			}
		}
		// Codigo lectura TCRT cada 10ms
		if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_A.flags, TCRT_NEW_VALUE)){
			SET_FLAG(IR_A.flags, TCRT_NEW_VALUE);
		}
		if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_B.flags, TCRT_NEW_VALUE)){
			SET_FLAG(IR_B.flags, TCRT_NEW_VALUE);
		}
		if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_C.flags, TCRT_NEW_VALUE)){
			SET_FLAG(IR_U.flags, TCRT_NEW_VALUE);
		}
		if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_U.flags, TCRT_NEW_VALUE)){
			SET_FLAG(IR_U.flags, TCRT_NEW_VALUE);
		}
	}
}
/* END Function ISR ----------------------------------------------------------*/

/* USART Functions -------------------------------------------------------*/
FILE mystdout = FDEV_SETUP_STREAM(USART_putchar_blocking, NULL, _FDEV_SETUP_WRITE); // Redirige printf (stdout) usando la función nativa de transmisión bloqueante.
FILE mystdin = FDEV_SETUP_STREAM(NULL, USART_getchar, _FDEV_SETUP_READ); // Redirige la entrada (stdin) usando una función personalizada que lea de tu buffer RX.
/* END USART Functions -------------------------------------------------------*/


/* Timer and External Interrupt Functions ---------------------------------*/
/**
 * @brief Inicializa el Timer 1 (timer1_init)
 *
 * Configura el Timer 1 para manejar:
 * - Captura de entrada (Input Capture) para medición del sensor ultrasónico.
 * - Interrupción por desbordamiento (Overflow) para ampliar la resolución del temporizador.
 * - Interrupciones de comparación A y B para el control de servos.
 *
 * @details
 * - Se configura el Timer en modo normal.
 * - Se activa la captura en flanco ascendente inicialmente.
 * - Se establece un prescaler de 8 para una buena resolución temporal.
 * - Se carga `OCR1A` para la primera interrupción de servo en 20ms.
 *
 * @retval No aplica (void)
 */
void timer1_init()
{
	// Reset the timer counter
	TCNT1 = 0;
	// Clear input capture flag
	TIFR1 |= (1 << ICF1);
	// Enable input capture interrupt and overflow interrupt
	TIMSK1 |= (1 << ICIE1) | (1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B);
	// Set to capture rising edge initially
	TCCR1B |= (1 << ICES1);
	// Set prescaler to 8 for adequate timing resolution
	TCCR1B |= (1 << CS11);  // Prescaler = 8
	OCR1A = TCNT1 + SERVO_FRAME_PERIOD; // Primera interrupción en 20ms desde ya
}

/**
 * @brief Inicializa los pines GPIO del sistema (gpio_pins_init)
 *
 * Configura la dirección y el estado inicial de los pines de entrada/salida
 * utilizados por el sistema: LED, TRIGGER, ECHO, botón, y pines de servos.
 *
 * @details
 * - LED configurado como salida y encendido.
 * - TRIGGER como salida, ECHO como entrada.
 * - Botón como entrada.
 * - Pines de servo (A, B, C) como salidas.
 * - Se inicializa el sensor TCRT con `tcrt_init()`.
 * - Se garantiza que TRIGGER y BUTTON estén en bajo inicialmente.
 *
 * @retval No aplica (void)
 */
void timer2_init()
{
	// Configura el Timer 2 en modo CTC (Clear Timer on Compare Match)
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler de 1024
	OCR2A = 155;  // Para generar una interrupción cada 10 ms (156 ciclos de temporizador)
	TIMSK2 |= (1 << OCIE2A);  // Habilita la interrupción por comparación de Timer 2
	TCNT2 = 0;  // Inicializa el contador de Timer 2 en 0
	// El prescaler se configurará cuando necesitemos iniciar el timer
}
/* END Timer and External Interrupt Functions ------------------------------*/

/* Structural Inline Functions ---------------------------------*/

/**
 * @brief Calibra todos los sensores IR habilitados.
 *
 * Esta función inicia la calibración de los sensores IR (A, B, C, U) que estén marcados como habilitados.
 * La calibración se realiza una sola vez gracias a un flag interno `init_done`.
 * La función debe ser llamada periódicamente hasta que retorne `true`.
 *
 * @retval true  Si todos los sensores han terminado su calibración.
 * @retval false Si alguno aún está en proceso de calibración.
 *
 * @note Esta función está declarada como `inline` para permitir su expansión directa
 * en el lugar donde se use, lo cual mejora el rendimiento en este tipo de funciónes que son solo para estructurar el codigo.
 */
static inline bool calibrateAllIRSensors()
{
	static bool init_done = false;

	if (!init_done) {
		printf_P(PSTR("Calibrando sensores IR...\n"));
		if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED)){
			SET_FLAG(IR_A.flags, TCRT_CALIBRATING);
		}
		if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED)){
			SET_FLAG(IR_B.flags, TCRT_CALIBRATING);
		}
		if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED)){
			SET_FLAG(IR_C.flags, TCRT_CALIBRATING);
		}
		if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED)){
			SET_FLAG(IR_U.flags, TCRT_CALIBRATING);
		}
		init_done = true;
	}

	if (IS_FLAG_SET(IR_A.flags, TCRT_CALIBRATING)) {
		calibrateIRSensor(&IR_A);
	}
	if (IS_FLAG_SET(IR_B.flags, TCRT_CALIBRATING)) {
		calibrateIRSensor(&IR_B);
	}
	if (IS_FLAG_SET(IR_C.flags, TCRT_CALIBRATING)) {
		calibrateIRSensor(&IR_C);
	}
	if (IS_FLAG_SET(IR_U.flags, TCRT_CALIBRATING)) {
		calibrateIRSensor(&IR_U);
	}
	// Lo mismo con los demás si están habilitados...

	// Condición de salida
	if (!IS_FLAG_SET(IR_A.flags, TCRT_CALIBRATING) &&
	!IS_FLAG_SET(IR_B.flags, TCRT_CALIBRATING) &&
	!IS_FLAG_SET(IR_C.flags, TCRT_CALIBRATING) &&
	!IS_FLAG_SET(IR_U.flags, TCRT_CALIBRATING)) { //Si hay alguno no ENABLED, entonces no entra en CALIBRATING mode y aca puede salir tranquilo, no bloquea el funcionamiento no tener un sensor
		return true; // Listo
	}
	return false; // Sigue calibrando
}

/**
 * @brief Tarea asociada al botón de entrada.
 *
 * Verifica si el botón ha sido presionado y soltado, y determina si la duración fue suficiente.
 * Si se detecta una presión larga (con overflow del contador) también es aceptada como válida.
 *
 * @note Modifica las banderas `BTN_PRESSED`, `BTN_RELEASED` y `BTN_OVF`.
 *
 * @note Esta función está declarada como `inline` para optimizar el manejo rápido de eventos
 * relacionados al botón, que puede ser llamado con alta frecuencia dentro de un ciclo de tareas.
 */
static inline void buttonTask(void){
	if((PIND & (1 << BUTTON_PIN)) && !BTN_PRESSED){ //Presionado y no salto la flag aun
		btn_pressed_time = 0;
		BTN_PRESSED = 1;
		}else if(BTN_PRESSED && !(PIND & (1 << BUTTON_PIN))){ //Flag activa y no presionado, estuvo presionado y se solto
		BTN_PRESSED = 0;
		if(BTN_OVF){ //Se sostuvo demasiado y hubo un overflow en el contador
			printf_P(PSTR("Btn overflowed\n"));
		}
		if(btn_pressed_time >= BTN_PRESS_TIME || BTN_OVF){ //Overflow marcado como un tiempo valido
			BTN_RELEASED = 1;
			}else{ //Reiniciar
			btn_pressed_time = 0;
			if(BTN_OVF){
				BTN_OVF = 0;
			}
		}
	}
}

/**
 * @brief Inicializa los pines GPIO del sistema.
 *
 * Configura los pines de entrada/salida para los periféricos como LED, botón, trigger ultrasónico,
 * pines de servo y sensores TCRT. También se asegura de poner ciertos pines en bajo por seguridad.
 *
 * @note Esta función está declarada como `inline` porque se espera que se llame una única vez
 * al inicio del programa, y no tiene sentido agregarle sobrecarga de llamada.
 */
static inline void gpio_pins_init() {
	DDRB |= (1 << LED_BUILTIN_PIN);   // LED pin salida
	PORTB |= (1 << LED_BUILTIN_PIN);  // LED HIGH
	
	DDRD |= (1 << TRIGGER_PIN);       // TRIGGER pin salida
	DDRB &= ~(1 << ECHO_PIN);         // ECHO pin como entrada
	DDRD &= ~(1 << BUTTON_PIN);       // BUTTON pin como entrada
	
	//Inicializar las salidas de servos
	DDRB |= (1 << SERVOA_PIN); // SERVOA_PIN como salida
	DDRB |= (1 << SERVOB_PIN); // SERVOB_PIN como salida
	DDRB |= (1 << SERVOC_PIN); // SERVOC_PIN como salida
	
	tcrt_init(); //Despues haremos esto para todos los sensores, modularizando
	
	// Poner en bajo estos pines para asegurarnos
	PORTD &= ~(1 << TRIGGER_PIN);
	PORTD &= ~(1 << BUTTON_PIN);
}
/* END Structural Inline Functions  ------------------------------*/

//Funcion wrapper para la libreria, asi la libreria escribe desde adentro
/**
 * @brief Wrapper para imprimir mensajes por consola.
 *
 * Imprime el mensaje recibido utilizando `printf`. Está pensado para ser usado como función de callback
 * en bibliotecas externas que requieren una función con este formato para redirigir la salida.
 *
 * @param [in] message Cadena de texto terminada en NULL a imprimir.
 */
void printfWrapper(const char* message) {
	printf("%s\n", message);
}

int main()
{
	// Inicializa las banderas en 0
	bandera.byte = 0;
	bandera2.byte = 0;
	bandera3.byte = 0;
	diezMsCounter = 0;
	
	// Habilita el trigger y verifica explícitamente que otras banderas estén en 0
	TIMER2_ACTIVE = 1;
	ULTRASONIC_ENABLE = 1;
	
	DEBUG_FLAGS = 0; //Bandera de debug ultrasonido
	DEBUG_FLAGS_SORTER = 1; //Bandera de debug del clasificador
	DEBUG_FLAGS_SERVOS = 1; //Bandera de debug de los servos
	DEBUG_FLAGS_EEPROM = 1; //Bandera de debug de la eeprom
	
	USART_Init(8);  // Inicializa la comunicación serial a 115200 baudios para un reloj de 16 MHz
	stdout = &mystdout; // Redirigir la salida estándar a USART
	stdin = &mystdin; // Redirigir la entrada estándar a USART
	
	initProtocolService(&protocolService); //Iniciar servicio de protocolo
	
	gpio_pins_init(); 	// Inicializa los pines GPIO

	// Inicializa los temporizadores
	timer1_init();
	timer2_init();
	
	TCRT_init_Handlers(); //Inicializa las estructuras TCRT_X
	ultrasonic_init(&ultraSensor, printfWrapper); 	//Inicia HCSR04 en la libreria, pasa el metodo por el cual escribir en Serial
	ultrasonic_set_debug_mode(&ultraSensor, DEBUG_FLAGS ? true : false); //Establece bandera de debug
	initDetector(&hcsr04Detector, &ultraSensor, &IR_U); //Inicializar el dectector IR con el ultrasonico
	initSorter(&SorterSystem); //Iniciar el clasificador
	
	EMIT_TRIGGER = 1; //Solo si quiero emitir al iniciar, sino sacar
	
	//Imprime iniciado
	printf_P(PSTR("Iniciado\n"));
	
	sei(); //Inicia las interrupciones
	
	while (1)
	{ 
		if(IR_CALIBRATED){
			irSensorsTask(&SorterSystem);
		}else{
			if(calibrateAllIRSensors()){ //Etapa de calibracion de los sensores IR
				printf_P(PSTR("Todos los sensores IR calibrados.\n"));
				IR_CALIBRATED = 1;
				IR_A.calibrationCounter = 0;
				IR_B.calibrationCounter = 0;
				IR_C.calibrationCounter = 0;
				IR_U.calibrationCounter = 0;
				initOutputs(); //Recien aca inicializar las salidas, porque recien inicializamos los IR
			}
		}
		ultraSensorTask(&hcsr04Detector, &SorterSystem); //Recordar que la funcion pide un puntero y esto ya es un puntero, por lo que no lo apunto con &
		servosTask(); //Task de los servos
		buttonTask(); //Task del boton
		protocolTask(); //Task del protocolo
		if(DO_ACTION){ //Esto habilita o deshabilita las acciones de los comandos
			DO_ACTION = 0;
			printf_P(PSTR("Comando aceptado \n"));
			printCommandMessage(doActionCmd);
			doAction(doActionCmd);
		}
		if(NEW_CONFIG){ //Bandera de cambio de configuracion en EEPROM
			NEW_CONFIG = 0;
			currentConfig.salidaA = salidaA.boxType;
			currentConfig.salidaB = salidaB.boxType;
			currentConfig.salidaC = salidaC.boxType;
			saveConfigurationRAM(&currentConfig);
		}
		if(WAIT_TIME_TRIGGER_PASSED){ //Esta bandera salta cuando se cunplio el tiempo de espera entre triggers
			WAIT_TIME_TRIGGER_PASSED = 0;
			ultraSensor.TRIGGER_ALLOWED = 1;
		}
		if(ECHO_INTERVAL_FLAG){ //Esto controla cuando entra a emitir otro trigger, se activa en el ISR del Timer2
			ECHO_INTERVAL_FLAG = 0;
			EMIT_TRIGGER = 1; //Activar emision del trigger
		}
		
		if(BTN_RELEASED){ //Bandera que controla accion del press del boton
			BTN_RELEASED = 0; 
		}
		if(IR_READ_INTERRUPT){ //Bandera de calibracion de los IR
			IR_READ_INTERRUPT = 0;
			if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_A.flags, TCRT_CALIBRATING) && !IS_FLAG_SET(IR_A.flags, TCRT_NEW_VALUE)){ //Esta calibrando sensor IR A
				SET_FLAG(IR_A.flags, TCRT_NEW_VALUE); //Setear la toma de nuevo valor
			}
			if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_B.flags, TCRT_CALIBRATING) && !IS_FLAG_SET(IR_B.flags, TCRT_NEW_VALUE)){ //Esta calibrando sensor IR B
				SET_FLAG(IR_B.flags, TCRT_NEW_VALUE); //Setear la toma de nuevo valor
			}
			if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_C.flags, TCRT_CALIBRATING) && !IS_FLAG_SET(IR_C.flags, TCRT_NEW_VALUE)){ //Esta calibrando sensor IR C
				SET_FLAG(IR_C.flags, TCRT_NEW_VALUE); //Setear la toma de nuevo valor
			}
			if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_U.flags, TCRT_CALIBRATING) && !IS_FLAG_SET(IR_U.flags, TCRT_NEW_VALUE)){ //Esta calibrando sensor IR U
				SET_FLAG(IR_U.flags, TCRT_NEW_VALUE); //Setear la toma de nuevo valor
			}
		}
	}

	return 0;
}