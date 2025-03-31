/*
 * USART_UTILS.c
 *
 * Created: 22-Mar-25 5:35:40 PM
 *  Author: kobac
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "./../../main.h"
#include "./../../types/protocolTypes.h"

/* USART Functions -----------------------------------------------------------*/
// Funci�n para inicializar el puerto serial (USART)
void USART_Init(uint16_t ubrr)
{
	// Configurar el baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;

	// Habilitar receptor, transmisor y la interrupci�n de recepci�n
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	
	// Configurar el formato: 8 bits de datos, sin paridad, 1 bit de stop
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Funci�n para enviar un car�cter por serial
void USART_Transmit(unsigned char data)
{
	// Espera hasta que el buffer de transmisi�n est� vac�o
	while (!(UCSR0A & (1 << UDRE0)));
	// Env�a el dato al registro de transmisi�n (UDR0)
	UDR0 = data;
}

// Funci�n para recibir un car�cter por serial
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
	// Si se desea manejar la nueva l�nea con retorno de carro:
	if (c == '\n')
	USART_putchar('\r', stream);
	
	// Esperar a que el buffer de transmisi�n (hardware) est� vac�o
	while (!(UCSR0A & (1 << UDRE0)))
	;  // Espera activa
	
	// Enviar el car�cter
	UDR0 = c;
	return 0;
}


int USART_getchar(FILE *stream)
{
	// Si no hay datos disponibles, se retorna -1 (puedes interpretar esto como EOF)
	if (protocolService.indexR == protocolService.indexW)
	return -1;
	
	uint8_t data = protocolService.buffer[protocolService.indexR];
	protocolService.indexR = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	return data;
}
