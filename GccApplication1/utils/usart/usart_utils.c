/*
 * USART_UTILS.c
 *
 * Created: 22-Mar-25 5:35:40 PM
 *  Author: kobac
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "./../../main.h"

/* USART Functions -----------------------------------------------------------*/
// Función para inicializar el puerto serial (USART)
void USART_Init(uint16_t ubrr)
{
	// Configura el baud rate para 115200 baudios
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
	// Envía el dato al registro de transmisión (UDR0)
	UDR0 = data;
}

// Función para recibir un carácter por serial
unsigned char USART_Receive(void)
{
	// Espera hasta que se haya recibido un dato
	while (!(UCSR0A & (1 << RXC0)));
	// Devuelve el dato recibido desde el registro de datos (UDR0)
	return UDR0;
}

// Redirige printf para usar USART
int USART_putchar(char c, FILE *stream)
{
	if (c == '\n')
	USART_Transmit('\r'); // Enviar un retorno de carro para manejar las nuevas líneas
	USART_Transmit(c);
	return 0;
}

// Redirige la entrada estándar (stdin) para recibir datos desde USART
int USART_getchar(FILE *stream)
{
	return USART_Receive();  // Recibir un carácter desde USART
}