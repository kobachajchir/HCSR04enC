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
/**
 * @brief Inicializa el periférico USART con el baud rate especificado.
 *
 * Configura los registros UBRR0H y UBRR0L para establecer la velocidad.
 * Habilita el transmisor, receptor y la interrupción de recepción.
 * Establece el formato: 8 bits de datos, sin paridad, 1 bit de stop.
 *
 * @param [in] ubrr Valor UBRR calculado para el baud rate deseado.
 */
void USART_Init(uint16_t ubrr)
{
	// Configurar el baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;

	// Habilitar receptor, transmisor y la interrupción de recepción
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	
	// Configurar el formato: 8 bits de datos, sin paridad, 1 bit de stop
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Función para enviar un byte: Encola el byte y activa la interrupción de transmisión
/*
 * Función comentada: USART_putchar
 *
 * En caso de ser utilizada, esta función permitiría enviar un byte a través
 * del buffer circular, activando la interrupción para transmitir.
 *
 * @param [in] c Byte a enviar.
 * @param [in] stream Stream usado por la librería stdio.
 * @retval 0 Si el byte fue encolado correctamente.
 */
// int USART_putchar(char c, FILE *stream) {
// 	uint8_t next_indexW = (protocolService.indexW + 1) % PROTOCOL_BUFFER_SIZE;
// 
// 	// Verifica si el buffer está lleno (reserva un espacio para diferenciar vacío de lleno)
// 	if (next_indexW == protocolService.indexR) {
// 		return 0; // El buffer está lleno, se puede manejar el error aquí si es necesario
// 	}
// 
// 	// Almacena el byte en el buffer y actualiza el índice de escritura
// 	protocolService.buffer[protocolService.indexW] = c;
// 	protocolService.indexW = next_indexW;
// 
// 	// Habilita la interrupción de transmisión para iniciar el envío
// 	UCSR0B |= (1 << UDRIE0);
// 	return 0;
// }

// Función bloqueante para enviar un byte por USART
/**
 * @brief Envía un carácter por USART de forma bloqueante.
 *
 * Espera a que el buffer de transmisión esté disponible antes de escribir el byte.
 *
 * @param [in] c Carácter a enviar.
 * @param [in] stream Stream de salida usado por printf.
 *
 * @retval 0 Éxito al enviar.
 */
int USART_putchar_blocking(char c, FILE *stream) {
	// Espera a que el registro de datos esté vacío (bit UDRE0 activo)
	while (!(UCSR0A & (1 << UDRE0))) {
		// Espera activa (polling)
	}
	UDR0 = c; // Envía el byte
	return 0;
}

/**
 * @brief Lee un byte desde el buffer de recepción circular.
 *
 * Verifica si hay datos disponibles en el buffer y los retorna. En caso contrario,
 * devuelve -1.
 *
 * @param [in] stream Stream de entrada (no se usa internamente).
 *
 * @retval Byte leído, o -1 si no hay datos disponibles.
 */
int USART_getchar(FILE *stream) {
	// Verifica si hay datos disponibles
	if (protocolService.indexR == protocolService.indexW) {
		return -1; // No hay datos disponibles
	}

	uint8_t data = protocolService.buffer[protocolService.indexR];
	protocolService.indexR = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	return data;
}
