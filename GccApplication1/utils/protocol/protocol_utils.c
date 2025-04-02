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
	clear_receive_pck();
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
	// Verificar si hay al menos 4 bytes disponibles
	uint8_t available;
	if (protocolService.indexW >= protocolService.indexR) {
		available = protocolService.indexW - protocolService.indexR;
		} else {
		available = PROTOCOL_BUFFER_SIZE - protocolService.indexR + protocolService.indexW;
	}

	// Comprobar que el primer byte es 'U'
	if (protocolService.buffer[protocolService.indexR] != 'U') {
		return false; // No es un header candidato.
	}

	// Calcular los índices para los 4 bytes del header, considerando el buffer circular.
	uint8_t idx0 = protocolService.indexR;
	uint8_t idx1 = (protocolService.indexR + 1) % PROTOCOL_BUFFER_SIZE;
	uint8_t idx2 = (protocolService.indexR + 2) % PROTOCOL_BUFFER_SIZE;
	uint8_t idx3 = (protocolService.indexR + 3) % PROTOCOL_BUFFER_SIZE;

	// Calcular la suma ponderada de los 4 bytes usando pesos: 4, 3, 2, 1
	uint16_t sum = 0;
	sum += (uint16_t)protocolService.buffer[idx0] * 4;  // 'U'
	sum += (uint16_t)protocolService.buffer[idx1] * 3;  // 'N'
	sum += (uint16_t)protocolService.buffer[idx2] * 2;  // 'E'
	sum += (uint16_t)protocolService.buffer[idx3] * 1;  // 'R'
	
	// Reducir a 8 bits: tomar los 8 bits menos significativos
	uint8_t computed = (uint8_t)(sum & 0xFF);

	// Comparar con el valor esperado
	if (computed == EXPECTED_HEADER_SUM) {
		protocolService.indexR = idx3; // Avanzar indexR hasta la posición del cuarto byte (donde debería estar 'R')
		return true;  // La cabecera es válida.
	} else {
		// No coincide; descartar este header.
		return false;
	}
}

bool process_protocol_buffer() {
	uint8_t available;
	if (protocolService.indexW >= protocolService.indexR) {
		available = protocolService.indexW - protocolService.indexR;
		} else {
		available = PROTOCOL_BUFFER_SIZE - protocolService.indexR + protocolService.indexW;
	}

	if (available < PROTOCOL_MIN_BYTE_COUNT) {
		printf("Menos de %u bytes en buffer\n", PROTOCOL_MIN_BYTE_COUNT);
		return false;
	}
	
	if (verify_header()) {
		// Se continúa procesando el paquete...
		return true;
		} else {
		return false;
	}
}

Command getResponseCommand(Command req) {
	for (size_t i = 0; i < NUM_COMMANDS; i++) {
		if (commandMap[i].request == req) {
			return commandMap[i].response;
		}
	}
	// En caso de no encontrar coincidencia, se retorna CMD_ALIVE como valor por defecto
	return CMD_ALIVE;
}

void clear_receive_pck(){
	protocolService.receivePck.checksum = 0;
	protocolService.receivePck.cmd = 0;
	protocolService.receivePck.length = 0;
	protocolService.receivePck.token = 0;
	protocolService.receivePck.payload[0] = NULL;
	protocolService.receivePck.header[0] = NULL;
	printf("Paquete RCV cleared \n");
}

uint8_t calculatePayload() {
	uint8_t checksum = 0;  // Inicializa el checksum en 0
	
	// Sumar los bytes de 'UNER'
	checksum ^= 'U';  // XOR de 'U'
	checksum ^= 'N';  // XOR de 'N'
	checksum ^= 'E';  // XOR de 'E'
	checksum ^= 'R';  // XOR de 'R'
	
	// Sumar el byte de LENGTH
	checksum ^= protocolService.receivePck.length;
	
	// Sumar el byte de TOKEN (usamos el valor de PROTOCOL_TOKEN)
	checksum ^= PROTOCOL_TOKEN;
	
	// Sumar el byte de CMD
	checksum ^= protocolService.receivePck.cmd;
	
	// Imprimir los valores del payload
	printf("Payload en Hex:\n");
	// CHANGED: Removed the -1 to process all bytes
	for (int i = 0; i < protocolService.receivePck.length; i++) {
		uint8_t payload_byte = *(protocolService.receivePck.payload + i);
		printf("Byte %d: 0x%02X (Decimal: %d)\n", i, payload_byte, payload_byte);
		checksum ^= payload_byte;  // XOR de cada byte del payload
	}
	
	// Imprimir el checksum calculado en hexadecimal
	printf("CHECKSUM CALC %02X\n", checksum);
	
	// Imprimir el checksum esperado
	printf("CHECKSUM ESPERADO: %02X\n", protocolService.receivePck.checksum);
	
	// Comparar el checksum calculado con el esperado
	if (checksum == protocolService.receivePck.checksum) {
		printf("Cks valido\n");
		} else {
		printf("Cks invalido\n");
	}
	
	// Devuelve el checksum calculado
	return checksum;
}

void createPck(uint8_t cmd, uint8_t* payload, uint8_t payloadLength) {
	// Fill the header with 'UNER'
	protocolService.receivePck.header[0] = 'U';
	protocolService.receivePck.header[1] = 'N';
	protocolService.receivePck.header[2] = 'E';
	protocolService.receivePck.header[3] = 'R';
	
	// Set the length field to the payload length
	protocolService.receivePck.length = payloadLength;
	
	// Set the token
	protocolService.receivePck.token = PROTOCOL_TOKEN; // Assuming PROTOCOL_TOKEN is defined as 0x3A
	
	// Set the command
	protocolService.receivePck.cmd = cmd;
	
	// Point the payload to the correct position in the buffer
	// protocolService.receivePck.payload is already pointing to the right memory location
	// as mentioned in the description
	
	// Calculate the checksum
	uint8_t checksum = 0;
	
	// XOR with header bytes
	checksum ^= protocolService.receivePck.header[0]; // 'U'
	checksum ^= protocolService.receivePck.header[1]; // 'N'
	checksum ^= protocolService.receivePck.header[2]; // 'E'
	checksum ^= protocolService.receivePck.header[3]; // 'R'
	
	// XOR with length
	checksum ^= protocolService.receivePck.length;
	
	// XOR with token
	checksum ^= protocolService.receivePck.token;
	
	// XOR with command
	checksum ^= protocolService.receivePck.cmd;
	
	// XOR with each byte of the payload
	for (int i = 0; i < protocolService.receivePck.length; i++) {
		checksum ^= *(protocolService.receivePck.payload + i);
	}
	
	// Set the checksum field
	protocolService.receivePck.checksum = checksum;
	printf("Paquete creado \n");
}