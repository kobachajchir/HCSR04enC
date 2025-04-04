/*
 * protocol_utils.c
 *
 * Created: 30-Mar-25 7:50:36 PM
 *  Author: kobac
 */ 
#include "../../main.h"
#include "protocol_utils.h"
#include "../usart/usart_utils.h"
#include "../../types/boxTypes.h"
#include <string.h>
#include <avr/pgmspace.h>


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
	uint8_t reqValue = (uint8_t)req;
	uint8_t responseValue;
	
	switch(reqValue) {
		case CMD_START:
		responseValue = CMD_RESPONSE_START;
		break;
		
		case CMD_STOP:
		responseValue = CMD_RESPONSE_STOP;
		break;
		
		case CMD_SET_CONFIG:
		responseValue = CMD_RESPONSE_SET_CONFIG;
		break;
		
		case CMD_GET_CONFIG:
		responseValue = CMD_RESPONSE_GET_CONFIG;
		break;
		
		case CMD_GET_STATS:
		responseValue = CMD_RESPONSE_GET_STATS;
		break;
		
		case CMD_CLEAR_STATS:
		responseValue = CMD_RESPONSE_CLEAR_STATS;
		break;
		
		case CMD_GET_FIRMWARE:
		responseValue = CMD_RESPONSE_GET_FIRMWARE;
		break;
		
		case CMD_GET_REPOSITORY:
		responseValue = CMD_RESPONSE_GET_REPOSITORY;
		break;
		
		case CMD_ALIVE:
		responseValue = CMD_ALIVE;
		break;
		
		default:
		responseValue = CMD_INVALID;
		break;
	}
	
	printf("DEBUG: Request: 0x%X -> Response: 0x%X\n", reqValue, responseValue);
	return (Command)responseValue;
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

	// XOR de los bytes del HEADER "UNER"
	checksum ^= 'U';  // 0x55
	checksum ^= 'N';  // 0x4E
	checksum ^= 'E';  // 0x45
	checksum ^= 'R';  // 0x52

	// XOR con el campo LENGTH (que ahora es solo la cantidad de bytes del payload)
	checksum ^= protocolService.receivePck.length;

	// XOR con el TOKEN (se usa PROTOCOL_TOKEN, ej. ':' = 58)
	checksum ^= PROTOCOL_TOKEN;

	// XOR con el CMD
	checksum ^= protocolService.receivePck.cmd;

	// XOR con cada byte del PAYLOAD (cantidad EXACTA: LENGTH bytes)
	printf_P(PSTR("Payload en Hex:\n"));
	for (int i = 0; i < protocolService.receivePck.length; i++) {
		uint8_t payload_byte = *(protocolService.receivePck.payload + i);
		printf("Byte %d: 0x%02X (Decimal: %d)\n", i, payload_byte, payload_byte);
		checksum ^= payload_byte;
	}

	printf("CHECKSUM CALC %02X\n", checksum);
	printf("CHECKSUM ESPERADO: %02X\n", protocolService.receivePck.checksum);

	if (checksum == protocolService.receivePck.checksum) {
		printf_P(PSTR("Cks valido\n"));
		} else {
		printf_P(PSTR("Cks invalido\n"));
	}

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
	
	if(payload != NULL){
		protocolService.receivePck.payload = payload;	
	}
	
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
	printf("Valor CMD paquete creado %x \n", protocolService.receivePck.cmd);
}

bool validatePck(){
	// Se asume que el HEADER ("UNER") ya fue procesado o se encuentra en las primeras 4 posiciones
	// y que protocolService.indexR apunta a la última posición del HEADER (índice 3).
	// Luego se procede a leer el resto del paquete.

	// Leer LENGTH (payload length) en posición 4
	NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_READING_LEN);
	protocolService.indexR++; // Ahora indexR == 4
	protocolService.receivePck.length = protocolService.buffer[protocolService.indexR];

	// Leer TOKEN en posición 5
	NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_READING_TOKEN);
	protocolService.indexR++; // Ahora indexR == 5
	if (protocolService.buffer[protocolService.indexR] != PROTOCOL_TOKEN){
		printf_P(PSTR("Token invalido\n"));
		return false;
		} else {
		// Leer CMD en posición 6
		NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_READING_CMD);
		protocolService.indexR++; // Ahora indexR == 6
		protocolService.receivePck.cmd = protocolService.buffer[protocolService.indexR];
		if(protocolService.receivePck.cmd == CMD_INVALID){
			printf_P(PSTR("Comando invalido\n"));
			return false;
			} else {
			printf("Length %u\n", protocolService.receivePck.length);
			if(protocolService.receivePck.length > PROTOCOL_MAX_BYTE_COUNT){
				printf_P(PSTR("Length mayor a %u, se perderia data del buffer\n"), PROTOCOL_MAX_BYTE_COUNT);
				return false;
			}
			// Leer PAYLOAD: el payload tiene exactamente LENGTH bytes
			NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_READING_PAYLOAD);
			protocolService.indexR++; // Ahora indexR == 7, inicio del payload
			protocolService.receivePck.payload = &protocolService.buffer[protocolService.indexR];
			// Avanza el índice en la cantidad exacta de payload
			protocolService.indexR += protocolService.receivePck.length;
			
			// Leer CHECKSUM, que está justo después del payload
			NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_READING_CHK);
			protocolService.receivePck.checksum = protocolService.buffer[protocolService.indexR];
			
			// Calcular y comparar el checksum
			NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_CALCULATING_CHK);
			if(calculatePayload() != protocolService.receivePck.checksum){
				printf_P(PSTR("Cks invalido\n"));
				return false;
				} else {
				printf_P(PSTR("Cks valido\n"));
				// Procesar el payload según el comando recibido
				NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_PROCESSING_PAYLOAD);
				switch(protocolService.receivePck.cmd){
					case CMD_ALIVE:
					{
						printf_P(PSTR("ALIVE.\n"));
						CREATE_RESPONSE_PCK = 1;
					}
					break;
					case CMD_START:
					case CMD_STOP:
					{
						// Se espera un byte de estado (ej. 0x01 para On, 0x00 para Off)
						printf("Status: %u\n", protocolService.receivePck.payload[0]);
						CREATE_RESPONSE_PCK = 1;
					}
					break;
					case CMD_GET_CONFIG:
					{
						CREATE_RESPONSE_PCK = 1;
						break;
					}
					case CMD_SET_CONFIG:
					{
						// Copiar el payload a un buffer temporal y agregar terminación nula.
						char configStr[PROTOCOL_MAX_BYTE_COUNT + 1];
						uint8_t i;
						for(i = 0; i < protocolService.receivePck.length && i < PROTOCOL_MAX_BYTE_COUNT; i++){
							configStr[i] = protocolService.receivePck.payload[i];
						}
						configStr[i] = '\0';

						printf_P(PSTR("Configuracion (raw): %s\n"), configStr);

						// Se espera el formato: "0:A-1:B-2:C" (o similar).
						// Se separa cada token usando '-' como delimitador.
						char *token;
						token = strtok(configStr, "-");
						while(token != NULL){
							// Cada token debe tener el formato "n:Letra", por ejemplo "0:A"
							char *colon = strchr(token, ':');
							if(colon != NULL){
								*colon = '\0'; // Separa el número de la letra
								int output = atoi(token);
								char boxTypeLetter = *(colon + 1);
								box_type_t boxType;
								switch(boxTypeLetter){
									case 'A': boxType = BOX_SIZE_A; break;
									case 'B': boxType = BOX_SIZE_B; break;
									case 'C': boxType = BOX_SIZE_C; break;
									default:  boxType = BOX_DISCARDED; break;
								}
								// Asignar el tipo de caja según el número de salida recibido:
								switch(output){
									case 0:
									salidaA.boxType = boxType;
									break;
									case 1:
									salidaB.boxType = boxType;
									break;
									case 2:
									salidaC.boxType = boxType;
									break;
									default:
									// Opcional: manejar números de salida inválidos
									break;
								}
								printf("Salida %d: Box Type = %u\n", output, boxType);
								CREATE_RESPONSE_PCK = 1;
								} else {
								printf_P(PSTR("Formato de token invalido: %s\n"), token);
							}
							token = strtok(NULL, "-");
						}
						break;
					}

					case CMD_GET_FIRMWARE:
					{
						CREATE_RESPONSE_PCK = 1;
					}
					break;
					case CMD_GET_STATS:
					printf_P(PSTR("Estadisticas: %s\n"), protocolService.receivePck.payload);
					break;
					case CMD_CLEAR_STATS:
						printf("Clear Stats status: %u\n", protocolService.receivePck.payload[0]);
						SorterSystem.stats.total_by_type_array[0] = 0;
						SorterSystem.stats.total_by_type_array[1] = 0;
						SorterSystem.stats.total_by_type_array[2] = 0;
						SorterSystem.stats.total_discarded = 0;
						SorterSystem.stats.total_measured = 0;
					break;
					case CMD_GET_REPOSITORY:
					{
						CREATE_RESPONSE_PCK = 1;
						break;
					}
					default:
					printf_P(PSTR("Payload generico: %s\n"), protocolService.receivePck.payload);
					break;
				}
				return true;
			}
		}
	}
}

uint8_t protocolTask(){
	if (IS_FLAG_SET(protocolService.flags, PROTOSERV_CHECKDATA) && !IS_FLAG_SET(protocolService.flags, PROTOSERV_PROCESSING)) {
		printf_P(PSTR("Procesar info\n"));
		if (process_protocol_buffer()) {
			NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_READING_HEADER);
			SET_FLAG(protocolService.flags, PROTOSERV_PROCESSING);
			} else {
			SET_FLAG(protocolService.flags, PROTOSERV_RESET);
		}
	}
	if(IS_FLAG_SET(protocolService.flags, PROTOSERV_RESET)){
		protocolService.indexR = protocolService.indexW;
		CLEAR_FLAG(protocolService.flags, PROTOSERV_PROCESSING);
		CLEAR_FLAG(protocolService.flags, PROTOSERV_RESET);
		CLEAR_FLAG(protocolService.flags, PROTOSERV_CHECKDATA);
		NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_IDLE);
		clear_receive_pck();
	}
	if(IS_FLAG_SET(protocolService.flags, PROTOSERV_PROCESSING)){
		if(validatePck()){
			//PROCESAR DATOS ACA
			NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_VALIDATED);
			CLEAR_FLAG(protocolService.flags, PROTOSERV_PROCESSING);
			//POR AHORA SIEMPRE CREA PAQUETE DE RESPUESTA, DESPUES PONEMOS UNA BANDERA
			}else{
			CLEAR_FLAG(protocolService.flags, PROTOSERV_PROCESSING);
			SET_FLAG(protocolService.flags, PROTOSERV_RESET);
		}
	}
	if(CREATE_RESPONSE_PCK){
		CREATE_RESPONSE_PCK = 0;
		protocolService.receivePck.cmd = (uint8_t)getResponseCommand(protocolService.receivePck.cmd); //Asignar comando de respuesta
		SET_FLAG(protocolService.flags, PROTOSERV_CREATE_PCK);
		uint8_t len = create_payload(protocolService.receivePck.cmd);
		protocolService.indexW = ((protocolService.indexW + len)% PROTOCOL_BUFFER_SIZE);
		createPck(protocolService.receivePck.cmd, &protocolService.buffer[protocolService.indexW], len);
		CLEAR_FLAG(protocolService.flags, PROTOSERV_CREATE_PCK);
		NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_SEND);
	}
	if(NIBBLEH_GET_STATE(protocolService.flags) == PROTOSERV_SEND){
		if (!IS_TRANSMITTING) {
			UCSR0B &= ~(1 << RXCIE0);  // Desactiva la interrupción de recepción
			IS_TRANSMITTING = 1;
			UCSR0B |= (1 << UDRIE0);   // Activa la interrupción de transmisión para iniciar el envío
		}
		NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_SEND_DONE);
	}
	if(NIBBLEH_GET_STATE(protocolService.flags) == PROTOSERV_SEND_DONE){
		NIBBLEH_SET_STATE(protocolService.flags, PROTOSERV_IDLE);
	}
}

uint8_t create_payload(Command cmd) {
	uint8_t payload_length = 0;
	// Posición de inicio en el buffer (donde se escribirá el payload)
	uint8_t start_index = protocolService.indexW;

	switch (cmd) {
		case CMD_RESPONSE_ALIVE: {
			// No hay payload para CMD_RESPONSE_ALIVE
			payload_length = 0;
			break;
		}
		
		case CMD_RESPONSE_CLEAR_STATS: {
			// Payload: 1 byte (0x01 para éxito)
			protocolService.buffer[start_index] = 0x01;
			payload_length = 1;
			break;
		}
		
		case CMD_RESPONSE_START:
		case CMD_RESPONSE_STOP: {
			// Payload: 1 byte (0x01 para start, 0x00 para stop)
			protocolService.buffer[start_index] = (cmd == CMD_RESPONSE_START) ? 0x01 : 0x00;
			payload_length = 1;
			break;
		}
		
		case CMD_RESPONSE_SET_CONFIG:
		{
			protocolService.buffer[start_index] = 0x01;
			payload_length = 1;
			break;
		}		
		case CMD_RESPONSE_GET_CONFIG: {
			
			protocolService.buffer[start_index] = 'A';
			protocolService.buffer[(start_index + 1) % PROTOCOL_BUFFER_SIZE] = ':';
			protocolService.buffer[(start_index + 2) % PROTOCOL_BUFFER_SIZE] = (uint8_t)salidaA.boxType + '0';
			protocolService.buffer[(start_index + 3) % PROTOCOL_BUFFER_SIZE] = 'B';
			protocolService.buffer[(start_index + 4) % PROTOCOL_BUFFER_SIZE] = ':';
			protocolService.buffer[(start_index + 5) % PROTOCOL_BUFFER_SIZE] = (uint8_t)salidaB.boxType + '0';
			protocolService.buffer[(start_index + 6) % PROTOCOL_BUFFER_SIZE] = 'C';
			protocolService.buffer[(start_index + 7) % PROTOCOL_BUFFER_SIZE] = ':';
			protocolService.buffer[(start_index + 8) % PROTOCOL_BUFFER_SIZE] = (uint8_t)salidaC.boxType + '0';
	
			payload_length = 9;
			break;
		}
		
		case CMD_RESPONSE_GET_FIRMWARE: {	
			const char* firmware_version = DEV_FIRMWARE_VERSION;
			uint8_t i = 0;
			
			while (firmware_version[i] != '\0') {
				protocolService.buffer[(start_index + i) % PROTOCOL_BUFFER_SIZE] = firmware_version[i];
				i++;
			}
			payload_length = i;
			break;
		}
		
		case CMD_RESPONSE_GET_REPOSITORY: {
			const char* repo_url = DEV_REPOSITORY;
			uint8_t i = 0;
			
			while (repo_url[i] != '\0') {
				protocolService.buffer[(start_index + i) % PROTOCOL_BUFFER_SIZE] = repo_url[i];
				i++;
			}
			payload_length = i;
			break;
		}
		
		default: {
			// Comando inválido, sin payload
			payload_length = 0;
			break;
		}
	}
	return payload_length;
}
