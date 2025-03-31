/*
 * protocol_utils.c
 *
 * Created: 30-Mar-25 7:50:36 PM
 *  Author: kobac
 */ 
#include "../../main.h"
#include "protocol_utils.h"
#include "../usart/usart_utils.h"

void initProtocolService(ProtocolService* service){
	service->buffer[0] = NULL;
	service->indexR = 0;
	service->indexW = 0;
	service->flags.byte = 0;
	printf("Init protocol service\n");
}

uint8_t calculate_checksum(ProtocolFrame* pak) {
	uint8_t checksum = 0;
	for (int i = 0; i < 4; i++) {
		checksum ^= pak->header[i];
	}
	checksum ^= pak->length;
	checksum ^= pak->token;
	checksum ^= pak->cmd;
	for (int i = 0; i < pak->length - 1; i++) {
		checksum ^= pak->payload[i];
	}
	return checksum;
}

void send_protocol_frame(ProtocolFrame* pak) {
// 	// Enviar HEADER
// 	for (int i = 0; i < 4; i++) {
// 		USART_Transmit(pak->header[i]);
// 	}
// 	// Enviar LENGTH
// 	USART_Transmit(pak->length);
// 	// Enviar TOKEN
// 	USART_Transmit(pak->token);
// 	// Enviar CMD
// 	USART_Transmit(pak->cmd);
// 	// Enviar PAYLOAD
// 	for (int i = 0; i < pak->length - 1; i++) {
// 		USART_Transmit(pak->payload[i]);
// 	}
// 	// Enviar CHECKSUM
// 	USART_Transmit(pak->checksum);
}

ProtocolFrame receive_protocol_frame() {
// 	ProtocolFrame pak;
// 	// Leer HEADER
// 	for (int i = 0; i < 4; i++) {
// 		pak.header[i] = USART_Receive();
// 	}
// 	// Leer LENGTH
// 	pak.length = USART_Receive();
// 	// Leer TOKEN
// 	pak.token = USART_Receive();
// 	// Leer CMD
// 	pak.cmd = USART_Receive();
// 	// Leer PAYLOAD
// 	pak.payload = malloc(pak.length - 1); // Allocate memory for payload
// 	for (int i = 0; i < pak.length - 1; i++) {
// 		pak.payload[i] = USART_Receive();
// 	}
// 	// Leer CHECKSUM
// 	pak.checksum = USART_Receive();
// 	// Verificar el checksum
// 	if (pak.checksum != calculate_checksum(&pak)) {
// 		printf("Error: Checksum inválido\n");
// 	}
// 	return pak;
}

bool verify_header() {
	// Asegurarse de que hay al menos 4 bytes disponibles (esto depende de la implementación del buffer)
	// Primero, comprobamos que el primer byte es 'U'
	if (protocolService.buffer[protocolService.indexR] != 'U') {
		return false; // No es un header candidato.
	}
	
	uint8_t idx0 = protocolService.indexR; //254
	uint8_t idx1 = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	uint8_t idx2 = (protocolService.indexR + 2) % PROTOCOL_BUFFER_SIZE;
	uint8_t idx3 = (protocolService.indexR + 3) % PROTOCOL_BUFFER_SIZE;

	// Calcular la suma ponderada de los 4 bytes usando pesos: 4,3,2,1
	uint16_t sum = 0;
	sum += (uint16_t)protocolService.buffer[idx0] * 4;  // 'U'
	sum += (uint16_t)protocolService.buffer[idx1] * 3;  // 'N'
	sum += (uint16_t)protocolService.buffer[idx2] * 2;  // 'E'
	sum += (uint16_t)protocolService.buffer[idx3] * 1;  // 'R'
	
	// Reducir a 8 bits (por ejemplo, tomando el valor menor significativo)
	uint8_t computed = (uint8_t)(sum & NIBBLE_L_MASK);
	
	// Comparar con el valor esperado
	if (computed == EXPECTED_HEADER_SUM) {
		printf("Computado %u, Esperaba %u", computed, EXPECTED_HEADER_SUM);
		return true;  // La cabecera es válida.
		} else {
		// No coincide; ajustar indexR para descartar este candidato.
		// Se posiciona en la ubicación del cuarto byte (U + 3), donde debería estar 'R'.
		protocolService.indexR = idx3;  // O, si es un buffer circular, aplicar módulo.
		return false;
	}
}

bool process_protocol_buffer() {
	// Verificar si hay al menos 4 bytes para el header
	// (la verificación de tamaño depende de tu implementación del buffer)
	if((protocolService.indexW - protocolService.indexR)<4){
		printf("Menos de 4 bytes en buffer\n");
		return false;
	}
	if (verify_header()) {
		printf("Header UNER válido\n");
		// Se continúa procesando el paquete...
		} else {
		printf("Header inválido, descartado\n");
		// Se puede reintentar leer otro candidato
	}
}