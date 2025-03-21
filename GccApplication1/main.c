/* Includes ------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
/* END Includes --------------------------------------------------------------*/

/* typedef -------------------------------------------------------------------*/
typedef struct{
	union{
		uint8_t bit0: 1;
		uint8_t bit1: 1;
		uint8_t bit2: 1;
		uint8_t bit3: 1;
		uint8_t bit4: 1;
		uint8_t bit5: 1;
		uint8_t bit6: 1;
		uint8_t bit7: 1;
	}bitmap;
	uint8_t byte;
}Byte_Flag_Struct;

/* END typedef ---------------------------------------------------------------*/

/* define --------------------------------------------------------------------*/
#define F_CPU 16000000UL  // Definir la frecuencia del reloj en 16 MHz
#define TRIGGER_PIN  PD2 // Pin de Trigger PIN Numero 2
#define ECHO_PIN     PD3 // Pin de Echo PIN Numero 3

#define DO_TRIGGER bandera.bitmap.bit0
#define TRIGGER_FINISH bandera.bitmap.bit1
#define ECHO_RISING bandera.bitmap.bit2
#define ECHO_STATE bandera.bitmap.bit3  // Usamos el bit 3 para el estado de ECHO
#define TRIGGER_STATE bandera.bitmap.bit4 // Usamos el bit 4 para el estado de TRIGGER
#define TRIGGER_ALLOWED bandera.bitmap.bit5 // Usamos el bit 5 para el allow del TRIGGER

/* END define ----------------------------------------------------------------*/


/* Global variables ----------------------------------------------------------*/
Byte_Flag_Struct bandera;  // Definido para manejar flags
volatile uint16_t echo_init_time = 0;  // Tiempo de inicio (flanco ascendente)
volatile uint16_t echo_finish_time = 0;    // Tiempo final (flanco descendente)
volatile uint32_t distance_mm = 0;      // Distancia en milímetros
volatile uint8_t ovf_count = 0; // Contador de desbordamientos del Timer 1
volatile uint8_t wait_time = 0; // Contador de desbordamientos del Timer 1
volatile uint8_t echo_state = 0; // Estado de la señal de eco (0: esperando flanco ascendente, 1: esperando flanco descendente)
/* END Global variables ------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void timer1_init();
void timer2_init();
void external_interrupt_init();
void USART_Init(uint16_t ubrr);
void USART_Transmit(unsigned char data);
int USART_putchar(char c, FILE *stream);
void calculate_distance();
/* END Function prototypes ---------------------------------------------------*/


/* Function ISR --------------------------------------------------------------*/
ISR(TIMER1_CAPT_vect)
{
	// Se ejecuta cuando se detecta un flanco en el pin ECHO (ICP1)
	if (ECHO_RISING) {  // Si esperamos flanco ascendente (rising edge)
		// Guardamos el tiempo inicial cuando el eco llega (flanco ascendente)
		echo_init_time = ICR1 + (ovf_count * 65536);  // Captura el valor del temporizador
		echo_state = 1;  // Cambiamos el estado para esperar el flanco descendente
		ECHO_RISING = 0;  // Cambiamos el flag para capturar el flanco descendente
		// Configuramos el modo para capturar el flanco descendente
		TCCR1B &= ~_BV(ICES1);  // Capturar en el flanco descendente
		} else {  // Flanco descendente (falling edge)
		// Guardamos el tiempo final cuando el eco regresa (flanco descendente)
		echo_finish_time = ICR1 + (ovf_count * 65536);  // Captura el valor del temporizador
		calculate_distance();  // Calculamos y mostramos la distancia
		echo_state = 0;  // Reseteamos el estado para esperar el próximo flanco ascendente
		// Configuramos el modo para capturar el flanco ascendente nuevamente
		TCCR1B |= _BV(ICES1);  // Capturar en el flanco ascendente
	}
}


// ISR para el desbordamiento del Timer 1 (para contar los desbordamientos)
ISR(TIMER1_OVF_vect)
{
	ovf_count++;  // Incrementa el contador de desbordamientos del Timer 1
}


// ISR para Timer 2 (se ejecuta cada 10 ms)
ISR(TIMER2_COMPA_vect)
{
	// Solo setear las flags, no más procesamiento
	if(TRIGGER_STATE && !TRIGGER_FINISH){ //TRIGGER en alto y no finalizo
		TRIGGER_FINISH = 1; //Finalizar
	}else if(!TRIGGER_STATE){ //TRIGGER en alto y finalizo
		//nada
	}else if(!TRIGGER_STATE){ //TRIGGER en bajo
		if(!TRIGGER_ALLOWED && wait_time < 10){
			wait_time++;
		}else{
			TRIGGER_ALLOWED = 1;
			wait_time = 0;
		}
	}
}
/* END Function ISR ----------------------------------------------------------*/


/* USART Functions -----------------------------------------------------------*/
// Función para inicializar el puerto serial (USART)
void USART_Init(uint16_t ubrr)
{
	// Configura el baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);  // Parte alta del baud rate
	UBRR0L = (unsigned char)ubrr;         // Parte baja del baud rate
	// Habilita el receptor y transmisor
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);  // Habilitar RX y TX
	// Configura el formato de los datos: 8 bits de datos, sin paridad, 1 bit de stop
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Función para enviar un carácter por serial
void USART_Transmit(unsigned char data)
{
	// Espera hasta que el buffer de transmisión esté vacío
	while (!(UCSR0A & (1 << UDRE0)));
	// Envía el dato
	UDR0 = data;
}

// Redirige printf para usar USART
int USART_putchar(char c, FILE *stream)
{
	if (c == '\n')
	USART_Transmit('\r'); // Enviar un retorno de carro para manejar las nuevas líneas
	USART_Transmit(c);
	return 0;
}

// Inicializa la redirección de la salida estándar (stdout)
FILE mystdout = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
/* END USART Functions -------------------------------------------------------*/


/* Timer1 and External Interrupt Functions ---------------------------------*/
void timer1_init()
{
	// Configura el Timer 1 en modo normal con prescaler de 8
	TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler de 8
	TCNT1 = 0;  // Inicializa el contador en 0
	TIMSK1 |= (1 << TOIE1);  // Habilita la interrupción por desbordamiento del Timer 1
	// No activamos la captura de flancos al principio
}

void timer2_init()
{
	// Configura el Timer 2 en modo CTC (Clear Timer on Compare Match)
	TCCR2B |= (1 << WGM12) | (1 << CS22) | (1 << CS20);  // Modo CTC, prescaler de 1024
	OCR2A = 155;  // Para generar una interrupción cada 10 ms (156 ciclos de temporizador)
	TIMSK2 |= (1 << OCIE2A);  // Habilita la interrupción por comparación de Timer 2
	TCNT2 = 0;  // Inicializa el contador de Timer 2 en 0
}

// Configuración de interrupción externa (INT0) para el pin ECHO (flancos ascendentes y descendentes)
void external_interrupt_init()
{
	DDRD &= ~(1 << PD3);  // Configura el pin ECHO (PORTD3) como entrada
	DDRD |= (1 << PD2);   // Configura el pin TRIGGER (PORTD2) como salida

	// Configuración de interrupción externa
	EIMSK |= (1 << INT0);   // Habilita la interrupción externa INT0 (pin 2)
	EICRA |= (1 << ISC00);  // Configura INT0 para detectar flanco ascendente
	sei();                  // Habilita las interrupciones globales
}

void calculate_distance()
{
	// El tiempo de vuelo es la diferencia entre el tiempo final y el tiempo inicial
	uint32_t pulse_duration = echo_finish_time - echo_init_time;

	// Calcula la distancia en milímetros (sin usar flotantes)
	distance_mm = (pulse_duration * 34) / 200;  // Multiplicamos por 34 y dividimos entre 200

	// Mostrar los resultados
	printf("Distancia: %lu mm\n", distance_mm);
}
/* END Timer1 and External Interrupt Functions ------------------------------*/


int main()
{
	// Inicializar el puerto serial con una tasa de baudios de 115200
	USART_Init(8);  // Para 115200 baudios con un reloj de 16 MHz
	TRIGGER_ALLOWED = 1;
	// Redirigir la salida estándar a USART
	stdout = &mystdout;

	// Inicializa los temporizadores y las interrupciones
	timer1_init();  // Inicializa el Timer 1
	timer2_init();  // Inicializa el Timer 2
	external_interrupt_init();  // Inicializa la interrupción externa INT0 (flancos)
	if(TRIGGER_ALLOWED){
		DO_TRIGGER = 1;
	}

	while (1)
	{
		// Emitir el TRIGGER
		if (DO_TRIGGER && !TRIGGER_STATE)
		{
			DO_TRIGGER = 0;  // Resetea la bandera
			TRIGGER_STATE = 1;
			TRIGGER_FINISH = 0;
			// Configuramos Timer 1 para capturar el flanco ascendente después de emitir el trigger
			TCCR1B |= (1 << ICES1);  // Capturar en el flanco ascendente (ECHO_RISING)
			ECHO_RISING = 1;  // Indicamos que estamos esperando el flanco ascendente
			// Emitir el pulso TRIGGER (10 microsegundos)
			PORTD |= (1 << PD2);  // Pin TRIGGER a HIGH
		}else if(TRIGGER_STATE && TRIGGER_FINISH){
			TRIGGER_STATE = 0;
			PORTD &= ~(1 << PD2); // Pin TRIGGER a LOW
			TRIGGER_ALLOWED = 0;
		}

		// Aquí el código principal puede hacer otras tareas
		// La medición de distancia y las interrupciones se manejan en segundo plano
	}

	return 0;
}
