/* Includes ------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "main.h"
#include "types/bitmapType.h"
#include "types/cintaType.h"
#include "utils/usart/usart_utils.h"
#include "utils/servo/servo_utils.h"
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
volatile uint8_t cienMsCounter = 0; //Counter de 100ms
volatile uint8_t veintems_counter = 0;
uint32_t now = 0;
uint8_t servoAVal = 0; //Angulo 0 a 180 Servo A
cinta_out outA;
cinta_out outB;
cinta_out outC;
cinta_out outD;
ultrasonic_t ultraSensor;

/* END Global variables ------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
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
			ultraSensor.echo_init_time = (uint32_t)(ICR1 + (ovf_count * 65536UL));
			ultraSensor.ECHO_RISING = 1;
			break;
		}
		case ULTRA_WAIT_FALLING:
		{
			ultraSensor.echo_finish_time = (uint32_t)(ICR1 + (ovf_count * 65536UL));
			ultraSensor.CALCULATE_DISTANCE = 1;
			break;
		}
	}
}


// ISR para el desbordamiento del Timer 1 (para contar los desbordamientos)
ISR(TIMER1_OVF_vect)
{
	ovf_count++;  // Incrementa el contador de desbordamientos del Timer 1
}

// ISR para Compare A - inicia el pulso
ISR(TIMER1_COMPA_vect) {
	// Programar siguiente interrupción a 50Hz
	OCR1A += SERVO_FRAME_PERIOD;
	// Iniciar pulso
	PORTB |= (1 << SERVOA_PIN);
	// Programar Compare B para finalizar el pulso
	OCR1B = TCNT1 + pulse_width_A;
}

// ISR para Compare B - finaliza el pulso
ISR(TIMER1_COMPB_vect) {
	// Finalizar pulso
	PORTB &= ~(1 << SERVOA_PIN);
}

// ISR para Timer 2 (se ejecuta cada 10 ms)
ISR(TIMER2_COMPA_vect)
{
	if(TIMER2_ACTIVE){
		// Lógica para manejar el estado del TRIGGER
		if(ultraSensor.state == ULTRA_TRIGGERING && ultraSensor.ECHO_RISING && !ultraSensor.TRIGGER_FINISH) { // TRIGGER en alto y no finalizo
			ultraSensor.TRIGGER_FINISH = 1; // Marcar como finalizado
		}else if((ultraSensor.state == ULTRA_WAIT_FALLING || ultraSensor.state == ULTRA_WAIT_RISING) && !VEINTEMS_PASSED){
			if(veintems_counter < 2){
				veintems_counter++;	
			}else{
				if((ultraSensor.state == ULTRA_WAIT_FALLING || ultraSensor.state == ULTRA_WAIT_RISING)){
					VEINTEMS_PASSED = 1;
					veintems_counter = 0;	
				}
			}
		} else if(!ultraSensor.state == ULTRA_TRIGGERING) { // TRIGGER en bajo
			if(!ultraSensor.TRIGGER_ALLOWED && wait_time < 7) { // No esta habilitado el trigger de nuevo, esperar 70ms
				wait_time++;
				} else {
				ultraSensor.TRIGGER_ALLOWED = 1;
				wait_time = 0;
			}
		}
		if(BTN_PRESSED){
			if(btn_pressed_time == 255){
				BTN_OVF = 1;
			}
			btn_pressed_time++;
		}
		if(cienMsCounter < ECHO_INTERVAL_TENMS){ //100 es 1seg
			cienMsCounter++;
		}else{
			SECPASSED = 1;
		}
		if(SERVOA_RESET){
			if(servo_counter < SERVO_RESET_TIME){
				servo_counter++;
			}else{
				servo_counter = 0;
				SERVOA_RESET = 0;
				servoA_set_angle(90);
			}
		}
	}
}
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
	DDRB |= (1 << LED_BUILTIN_PIN);   // LED pin as output
	PORTB |= (1 << LED_BUILTIN_PIN);  // LED HIGH
	
	DDRD |= (1 << TRIGGER_PIN);       // TRIGGER pin as output
	DDRB &= ~(1 << ECHO_PIN);         // ECHO pin as input (now on PORTB)
	DDRD &= ~(1 << BUTTON_PIN);       // BUTTON pin as input
	PORTD &= ~(1 << BUTTON_PIN);
	DDRB |= (1 << SERVOA_PIN);
	
	// Make sure TRIGGER starts LOW
	PORTD &= ~(1 << TRIGGER_PIN);
}

// Configuración de interrupción externa (INT0) para el pin ECHO
void external_interrupt_init()
{
	// Configuración de interrupción externa
	EICRA |= (1 << ISC01) | (1 << ISC00);  // Configura INT0 para detectar flanco ascendente
	EIMSK |= (1 << INT0);                   // Habilita la interrupción externa INT0 (pin 2)
}

/* END Timer1 and External Interrupt Functions ------------------------------*/


int main()
{
	// Inicializa las banderas
	bandera.byte = 0;
	// Habilita el trigger y verifica explícitamente que otras banderas estén en 0
	TIMER2_ACTIVE = 1;
	ULTRASONIC_ENABLE = 1;
	// Inicializa la comunicación serial primero
	USART_Init(8);  // 115200 baudios para un reloj de 16 MHz
	// Redirigir la salida estándar a USART
	stdout = &mystdout;
	// Redirigir la entrada estándar a USART
	stdin = &mystdin;
	printf("Iniciado\n");
	outA.cinta_struct_full_mem = 0; 
	outB.cinta_struct_full_mem = 0; 
	outC.cinta_struct_full_mem = 0; 
	outD.cinta_struct_full_mem = 0; 
	
	// Inicializa los pines GPIO
	gpio_pins_init();
	
	// Inicializa los temporizadores
	timer1_init();
	timer2_init();
	
	ultrasonic_init(&ultraSensor);
	
	// Inicializa la interrupción externa
	//external_interrupt_init();
	
	// Inicia el proceso activando DO_TRIGGER
	while (1)
	{
		if(ULTRASONIC_ENABLE && ultraSensor.TRIGGER_ALLOWED){
			ultrasonic_start(&ultraSensor, now);
		}
		ultrasonic_update(&ultraSensor, now);
		if(ultraSensor.state == ULTRA_TRIGGERING){
			if(!ultraSensor.DO_TRIGGER){
				//printf("Trigger lanzado");
			}else if(ultraSensor.TRIGGER_FINISH){
				printf("Trigger termino");
			}
		}
		if(ultraSensor.state == ULTRA_DONE){
			printf("Distancia %ul", ultrasonic_get_distance(&ultraSensor));
		}
		if(VEINTEMS_PASSED){
			ultrasonic_hal_echo_timeout(&ultraSensor);
			VEINTEMS_PASSED = 0;
		}
		if(ultraSensor.TIMEDOUT){
			printf("Ultrasonido perdio ECHO");
		}
		if((PIND & (1 << BUTTON_PIN)) && !BTN_PRESSED){ //Presionado y no salto la flag aun
			btn_pressed_time = 0;
			BTN_PRESSED = 1;
		}else if(BTN_PRESSED && !(PIND & (1 << BUTTON_PIN))){ //Flag activa y no presionado, estuvo presionado y se solto
			BTN_PRESSED = 0;
			if(BTN_OVF){
				printf("Btn overflowed");
			}
			if(btn_pressed_time >= BTN_PRESS_TIME || BTN_OVF){
				BTN_RELEASED = 1;
			}else{ //Reiniciar
				btn_pressed_time = 0;
				if(BTN_OVF){
					BTN_OVF = 0;
				}
			}
		}
		if(SECPASSED){
			SECPASSED = 0;
			cienMsCounter = 0;
		}
		if(BTN_RELEASED){
			BTN_RELEASED = 0; //TEST SERVO A
			SERVOA_MOVE = 1;
		}
		if(SERVOA_MOVE){
			SERVOA_MOVE = 0;
			servoA_set_angle(0);
			SERVOA_RESET = 1;
		}
		// Aquí el código principal puede hacer otras tareas
		// La medición de distancia y las interrupciones se manejan en segundo plano
	}

	return 0;
}