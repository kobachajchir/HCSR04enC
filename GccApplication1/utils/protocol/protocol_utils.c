/*
 * protocol_utils.c
 *
 * Created: 30-Mar-25 7:50:36 PM
 *  Author: kobac
 */ 
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
