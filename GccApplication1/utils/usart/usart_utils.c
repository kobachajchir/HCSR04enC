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

// Funci�n para enviar un byte: Encola el byte y activa la interrupci�n de transmisi�n
int USART_putchar(char c, FILE *stream) {
	uint8_t next_indexW = (protocolService.indexW + 1) % PROTOCOL_BUFFER_SIZE;

	// Verifica si el buffer est� lleno (reserva un espacio para diferenciar vac�o de lleno)
	if (next_indexW == protocolService.indexR) {
		return 0; // El buffer est� lleno, se puede manejar el error aqu� si es necesario
	}

	// Almacena el byte en el buffer y actualiza el �ndice de escritura
	protocolService.buffer[protocolService.indexW] = c;
	protocolService.indexW = next_indexW;

	// Habilita la interrupci�n de transmisi�n para iniciar el env�o
	UCSR0B |= (1 << UDRIE0);
	return 0;
}

// Funci�n bloqueante para enviar un byte por USART
int USART_putchar_blocking(char c, FILE *stream) {
	// Espera a que el registro de datos est� vac�o (bit UDRE0 activo)
	while (!(UCSR0A & (1 << UDRE0))) {
		// Espera activa (polling)
	}
	UDR0 = c; // Env�a el byte
	return 0;
}

int USART_getchar(FILE *stream) {
	// Verifica si hay datos disponibles
	if (protocolService.indexR == protocolService.indexW) {
		return -1; // No hay datos disponibles
	}

	uint8_t data = protocolService.buffer[protocolService.indexR];
	protocolService.indexR = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	return data;
}
