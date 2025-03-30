/* Includes ------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include "main.h"
#include "utils/macros_utils.h"
#include "types/bitmapType.h"
#include "types/sorterSystemTypes.h"
#include "utils/usart/usart_utils.h"
#include "utils/servo/servo_utils.h"
#include "types/ultrasonicDetectorType.h"
#include "types/TCRTType.h"
#include "utils/programPath/boxsorter_utils.h"
#include "ultrasonic.h"
#include "ultrasonic_hal.h"

/* END Includes --------------------------------------------------------------*/

/* typedef -------------------------------------------------------------------*/



/* END typedef ---------------------------------------------------------------*/

/* define --------------------------------------------------------------------*/


/* END define ----------------------------------------------------------------*/


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


/* END Global variables ------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void printfWrapper(const char* message);
void timer1_init();
void timer2_init();
void external_interrupt_init();
void gpio_pins_init();
/* END Function prototypes ---------------------------------------------------*/


/* Function ISR --------------------------------------------------------------*/
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


// ISR para el desbordamiento del Timer 1 (para contar los desbordamientos)
ISR(TIMER1_OVF_vect)
{
	ovf_count++;  // Incrementa el contador de desbordamientos del Timer 1
}

ISR(TIMER1_COMPA_vect) {
	// Schedule next Compare A interrupt in 20ms
	OCR1A += SERVO_FRAME_PERIOD;
	IR_READ_INTERRUPT = 1;
	
	// Process all servos in one 20ms frame
	for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
		// Update pulse duration for this servo
		if (IS_FLAG_SET(servosArray[i]->flags, SERVO_ENABLE)) {
			// Calculate pulse width based on state
			if (IS_FLAG_SET(servosArray[i]->flags, SERVO_PUSH) &&
			!IS_FLAG_SET(servosArray[i]->flags, SERVO_RESET)) {
				// If in push mode, use maximum pulse
				servosArray[i]->pulse_us = SERVO_MAX_PULSE;
				printf("ISR COMPA PUSH %d...\n", servosArray[i]->pin);
				} else {
				// Otherwise use the angle setting
				servosArray[i]->pulse_us = calculate_angle_pulseUs(servosArray[i]->angle);
			}

			// Set the pin high immediately if it's the first servo
			if (i == 0) {
				PORTB |= (1 << servosArray[i]->pin);
				// Schedule turn-off with Compare B
				OCR1B = TCNT1 + servosArray[i]->pulse_us;
				current_servo = 0; // Mark this as the active servo
			}
		}
	}
}

ISR(TIMER1_COMPB_vect) {
	// Turn off the current servo pin
	PORTB &= ~(1 << servosArray[current_servo]->pin);
	
	// Check if we need to reset after push
	if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_PUSH) &&
	IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_RESET)) {
		
		// Clear the push and reset flags
		CLEAR_FLAG(servosArray[current_servo]->flags, SERVO_RESET);
		CLEAR_FLAG(servosArray[current_servo]->flags, SERVO_PUSH);
		printf("ISR COMPB RESET %d...\n", servosArray[current_servo]->pin);
		// Return to idle angle
		servosArray[current_servo]->angle = SERVO_IDLE_ANGLE;
		servosArray[current_servo]->pulse_us = calculate_angle_pulseUs(servosArray[current_servo]);
		printf("Volvio a posicion IDLE\n");
	}
	
	// Move to next servo, but only if there are more servos to process in this frame
	current_servo++;
	if (current_servo < NUM_OUTPUTS) {
		// If there are more servos to process, activate the next one
		if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_ENABLE)) {
			// Turn on the pin for the next servo
			PORTB |= (1 << servosArray[current_servo]->pin);
			// Schedule turn-off
			OCR1B = TCNT1 + servosArray[current_servo]->pulse_us;
			} else {
			// If the next servo isn't enabled, skip to the one after
			// This is a recursive call to COMPB logic without the actual interrupt
			PORTB &= ~(1 << servosArray[current_servo]->pin); // Make sure the pin is low
			current_servo++;
			// Continue checking for the next enabled servo
			if (current_servo < NUM_OUTPUTS && IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_ENABLE)) {
				PORTB |= (1 << servosArray[current_servo]->pin);
				OCR1B = TCNT1 + servosArray[current_servo]->pulse_us;
			}
		}
	}
	// If current_servo >= NUM_OUTPUTS, we've processed all servos for this frame
}

// ISR(TIMER1_COMPA_vect) {
// 	// Update pulse duration based on current angle or push state
// 	    // Debug - print current flags
// 	OCR1B = TCNT1 + SERVO_START_PULSE; //Setearlo por si no entramos abajo, si entramos se sobrescribe
// 	if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_ENABLE)) {
// 		// If servo is enabled, process it
// 		if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_PUSH) &&
// 		!IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_RESET)) {
// 			// If in push mode, use maximum pulse (assuming push is at 180 degrees)
// 			servosArray[current_servo]->pulse_us = SERVO_MAX_PULSE;
// 			printf("ISR COMPA PUSH %d...\n", servosArray[current_servo]->pin);
// 			SET_FLAG(servosArray[current_servo]->flags, SERVO_RESET);
// 			printf("COMPA: Setting RESET flag for servo %d\n", servosArray[current_servo]->pin);
// 		} else {
// 			// Otherwise use the angle setting
// 			servosArray[current_servo]->pulse_us = calculate_angle_pulseUs(servosArray[current_servo]->angle);
// 		}
// 		
// 		// Activate the current servo pin
// 		PORTB |= (1 << servosArray[current_servo]->pin);
// 		
// 		// Schedule pulse end with Compare B
// 		OCR1B = TCNT1 + servosArray[current_servo]->pulse_us;
// 	} 
// 	// Schedule next Compare A interrupt in 20ms
// 	OCR1A += SERVO_FRAME_PERIOD;
// 	IR_READ_INTERRUPT = 1;
// }
// 
// // ISR Compare B
// ISR(TIMER1_COMPB_vect) {
// 	// Turn off the current servo pin
// 	PORTB &= ~(1 << servosArray[current_servo]->pin);
// 	
// 	// Check if we need to reset after push
// 	if (IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_PUSH) &&
// 	IS_FLAG_SET(servosArray[current_servo]->flags, SERVO_RESET)) {
// 		
// 		// Clear the push and reset flags
// 		CLEAR_FLAG(servosArray[current_servo]->flags, SERVO_RESET);
// 		CLEAR_FLAG(servosArray[current_servo]->flags, SERVO_PUSH);
// 		printf("ISR COMPB RESET %d...\n", servosArray[current_servo]->pin);
// 		// Return to idle angle
// 		//servosArray[current_servo]->angle = SERVO_IDLE_ANGLE;
// 	}
// 	
// 	// Move to next servo (cycles through all servos)
// 	current_servo = (current_servo + 1) % NUM_OUTPUTS;
// }

// ISR para Timer 2 (se ejecuta cada 10 ms)

/* END Function ISR ----------------------------------------------------------*/

// Inicializa la redirección de la salida estándar (stdout)
FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);

// Inicializa la redirección de la entrada estándar (stdin)
FILE mystdin = FDEV_SETUP_STREAM(NULL, USART_getchar, _FDEV_SETUP_READ);


/* END USART Functions -------------------------------------------------------*/


/* Timer1 and External Interrupt Functions ---------------------------------*/

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

void timer2_init()
{
	// Configura el Timer 2 en modo CTC (Clear Timer on Compare Match)
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler de 1024
	OCR2A = 155;  // Para generar una interrupción cada 10 ms (156 ciclos de temporizador)
	TIMSK2 |= (1 << OCIE2A);  // Habilita la interrupción por comparación de Timer 2
	TCNT2 = 0;  // Inicializa el contador de Timer 2 en 0
	// El prescaler se configurará cuando necesitemos iniciar el timer
}

void gpio_pins_init() {
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

// Configuración de interrupción externa (INT0) para el pin ECHO
void external_interrupt_init()
{
	// Configuración de interrupción externa
	EICRA |= (1 << ISC01) | (1 << ISC00);  // Configura INT0 para detectar flanco ascendente
	EIMSK |= (1 << INT0);                   // Habilita la interrupción externa INT0 (pin 2)
}

void printfWrapper(const char* message) {
	printf("%s\n", message);
}

/* END Timer1 and External Interrupt Functions ------------------------------*/

ISR(TIMER2_COMPA_vect)
{
	if(TIMER2_ACTIVE) {
		// --- Gestión del pulso de TRIGGER en ULTRA_TRIGGERING ---
		if(ultraSensor.state == ULTRA_TRIGGERING) {
			if(!TRIGGER_ACTIVE) {
				// Primera interrupción: activa el trigger y marca la bandera
				ultrasonic_hal_trigger_setHigh();
				ultraSensor.DO_TRIGGER = 1;
				TRIGGER_ACTIVE = 1;
			} else {
				// Segunda interrupción: desactiva el trigger y señala que terminó el pulso
				ultrasonic_hal_trigger_setLow();
				ultraSensor.TRIGGER_FINISH = 1;
				TRIGGER_ACTIVE = 0;
				EMIT_TRIGGER = 0;
				WAITING_ECHO = 1;
				ultraSensor.TRIGGER_ALLOWED = 0;
				ultraSensor.state = ULTRA_WAIT_RISING;
				ultraSensor.ECHO_RISING = 1; // Indica que se ha enviado el trigger y se espera ECHO
				ultraSensor.TRIGGER_FINISH = 0; // Limpiamos la bandera para evitar transiciones repetidas
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
			if(wait_time < 7){ // 7 * 10ms = 70ms
				wait_time++;
				} else {
				WAIT_TIME_TRIGGER_PASSED = 1;
				wait_time = 0;
			}
			if(ultraSensor.TIMEDOUT){
				ultrasonic_timeout_clear(&ultraSensor, DEBUG_FLAGS ? true : false);
			}
		}
		// --- Contador general (no modificado) ---
		if(diezMsCounter < ECHO_INTERVAL_TENMS){
			diezMsCounter++;
			} else {
			ECHO_INTERVAL_FLAG = 1;
			diezMsCounter = 0;
		}
		
		// --- Código adicional (botón, servo, etc.) ---
		if(BTN_PRESSED){
			if(btn_pressed_time == 255){
				BTN_OVF = 1;
			}
			btn_pressed_time++;
		}
		/*if(IS_FLAG_SET(servoA.flags, SERVO_PUSH)){
			printf("Servo in PUSH mode: state_time = %d\n", servoA.state_time);
			if(servoA.state_time < SERVO_ACTIVE_TIME){
				servoA.state_time++;  // Increment time spent in PUSH mode
				printf("Servo in PUSH mode: state_time = %d\n", servoA.state_time);
			} else {
				// If we have reached the active time, reset state_time and set RESET flag
				servoA.state_time = 0;
				SET_FLAG(servoA.flags, SERVO_RESET);  // Set reset flag
				printf("Servo RESET triggered: state_time = %d\n", servoA.state_time);
			}
		}*/
		// Codigo lectura TCRT cada 10ms
		if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_A.flags, TCRT_NEW_VALUE)){ //Cada 10ms
			SET_FLAG(IR_A.flags, TCRT_NEW_VALUE);
		}
		if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_B.flags, TCRT_NEW_VALUE)){ //Cada 10ms
			SET_FLAG(IR_B.flags, TCRT_NEW_VALUE);
		}
// 		if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_C.flags, TCRT_NEW_VALUE)){ //Cada 10ms
// 			SET_FLAG(IR_U.flags, TCRT_NEW_VALUE);
// 		}
// 		if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED) && !IS_FLAG_SET(IR_U.flags, TCRT_NEW_VALUE)){ //Cada 10ms
// 			SET_FLAG(IR_U.flags, TCRT_NEW_VALUE);
// 		}
	}
}

static inline bool calibrateAllIRSensors()
{
	static bool init_done = false;

	if (!init_done) {
		printf("Calibrando sensores IR...\n");
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
	if (IS_FLAG_SET(IR_B.flags, TCRT_CALIBRATING)) {
		calibrateIRSensor(&IR_B);
	}
	if (IS_FLAG_SET(IR_B.flags, TCRT_CALIBRATING)) {
		calibrateIRSensor(&IR_B);
	}
	// Lo mismo con los demás si están habilitados...

	// Condición de salida
	if (!IS_FLAG_SET(IR_A.flags, TCRT_CALIBRATING) &&
	!IS_FLAG_SET(IR_B.flags, TCRT_CALIBRATING)) { //Agregar los otros despues
		return true; // Listo
	}
	return false; // Sigue calibrando
}

static inline void buttonTask(void){
	if((PIND & (1 << BUTTON_PIN)) && !BTN_PRESSED){ //Presionado y no salto la flag aun
		btn_pressed_time = 0;
		BTN_PRESSED = 1;
		}else if(BTN_PRESSED && !(PIND & (1 << BUTTON_PIN))){ //Flag activa y no presionado, estuvo presionado y se solto
		BTN_PRESSED = 0;
		if(BTN_OVF){ //Se sostuvo demasiado y hubo un overflow en el contador
			printf("Btn overflowed\n");
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


int main()
{
	// Inicializa las banderas en 0
	bandera.byte = 0;
	bandera2.byte = 0;
	bandera3.byte = 0;
	// Habilita el trigger y verifica explícitamente que otras banderas estén en 0
	TIMER2_ACTIVE = 1;
	ULTRASONIC_ENABLE = 1;
	DEBUG_FLAGS = 0;
	DEBUG_FLAGS_SORTER = 1;
	IR_CALIBRATED = 0;
	// Inicializa la comunicación serial primero
	USART_Init(8);  // 115200 baudios para un reloj de 16 MHz
	// Redirigir la salida estándar a USART
	stdout = &mystdout;
	// Redirigir la entrada estándar a USART
	stdin = &mystdin;
	diezMsCounter = 0;
	// Inicializa los pines GPIO
	gpio_pins_init();
	// Inicializa los temporizadores
	timer1_init();
	timer2_init();
	//Inicia HCSR04
	TCRT_init_Handlers(); //Inicializa las estructuras TCRT_X
	ultrasonic_init(&ultraSensor, printfWrapper);
	ultrasonic_set_debug_mode(&ultraSensor, DEBUG_FLAGS ? true : false);
	initDetector(&hcsr04Detector, &ultraSensor, &IR_U);
	initSorter(&SorterSystem);
	// Inicializa la interrupción externa
	//external_interrupt_init();
	EMIT_TRIGGER = 1; //Solo si quiero emitir al iniciar, sino sacar
	//Imprime iniciado
	printf("Iniciado\n");
	sei();
	while (1)
	{ 
		if(IR_CALIBRATED){
			irSensorsTask(&SorterSystem);
		}else{
			if(calibrateAllIRSensors()){
				printf("Todos los sensores IR calibrados.\n");
				IR_CALIBRATED = 1;
				IR_A.calibrationCounter = 0;
				IR_B.calibrationCounter = 0;
				IR_C.calibrationCounter = 0;
				IR_U.calibrationCounter = 0;
				initOutputs();
			}
		}
		ultraSensorTask(&hcsr04Detector, &SorterSystem); //Recordar que la funcion pide un puntero y esto ya es un puntero, por lo que no lo apunto con &
		servosTask();
		buttonTask();
		if(WAIT_TIME_TRIGGER_PASSED){ //Esta bandera salta cuando se cunplio el tiempo de espera entre triggers
			WAIT_TIME_TRIGGER_PASSED = 0;
			ultraSensor.TRIGGER_ALLOWED = 1;
		}
		if(ECHO_INTERVAL_FLAG){ //Esto controla cuando entra a emitir otro trigger
			ECHO_INTERVAL_FLAG = 0;
			EMIT_TRIGGER = 1;
			//IR_READ = 1; //DEBUG
		}
		
		if(BTN_RELEASED){ //Bandera que controla accion del press del boton
			BTN_RELEASED = 0; //TEST SERVO A
			//EMIT_TRIGGER = 1;
		}
		if(IR_READ_INTERRUPT){
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
		// Aquí el código principal puede hacer otras tareas
		// La medición de distancia y las interrupciones se manejan en segundo plano
	}

	return 0;
}