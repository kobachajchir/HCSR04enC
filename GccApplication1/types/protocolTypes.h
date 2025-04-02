/*
 * protocolType.h
 *
 * Created: 30-Mar-25 7:47:19 PM
 *  Author: kobac
 */ 


#ifndef PROTOCOLTYPE_H_
#define PROTOCOLTYPE_H_

#include "bitmapType.h"
#include <stdbool.h>

#define PROTOCOL_BUFFER_SIZE 32
#define PROTOCOL_TOKEN ':'
#define EXPECTED_HEADER_SUM 0x1A

#define PROTOSERV_IDLE 0
#define PROTOSERV_READING_HEADER 1
#define PROTOSERV_READING_LEN 2 //encuentra : y pasa a reading cmd
#define PROTOSERV_READING_TOKEN 3
#define PROTOSERV_READING_CMD 4
#define PROTOSERV_READING_PAYLOAD 5
#define PROTOSERV_READING_CHK 6
#define PROTOSERV_CALCULATING_CHK 7
#define PROTOSERV_VALIDATED 8
#define PROTOSERV_SEND 9
#define PROTOSERV_SEND_DONE 10

#define PROTOSERV_PROCESSING BIT0_MASK
#define PROTOSERV_CHECKDATA BIT1_MASK
#define PROTOSERV_RESET BIT2_MASK
#define PROTOSERV_CREATE_PCK BIT3_MASK

typedef struct {
	uint8_t header[4];           // HEADER (4 bytes)
	uint8_t length;              // LENGTH (1 byte)
	uint8_t token;               // TOKEN (1 byte)
	uint8_t cmd;                 // CMD (1 byte)
	uint8_t* payload;            // PAYLOAD (N bytes) //Marca la posicion en el buffer de protoService
	uint8_t checksum;            // CHECKSUM (1 byte)
} ProtocolFrame;

typedef enum {
	CMD_ALIVE = 0xA0,       // Comando de respuesta (Alive)
	CMD_START = 0xA1,       // Comando de inicio
	CMD_STOP = 0xA2,        // Comando para detener alguna operación
	CMD_SET_CONFIG = 0xC0,  // Comando de configuración - setear
	CMD_GET_CONFIG = 0xC1,  // Comando de configuración - obtener
	CMD_GET_FIRMWARE = 0xF0, // Comando para obtener firmware
	CMD_INVALID = 0xE0, // Comando para obtener firmware
	// Agrega otros comandos según necesidad
} Command;

typedef struct {
	Command request;
	Command response;
} CommandMap;

static const CommandMap commandMap[] = {
	{ CMD_START,      CMD_ALIVE },
	{ CMD_STOP,       CMD_ALIVE },
	{ CMD_SET_CONFIG, CMD_ALIVE },
	{ CMD_GET_CONFIG, CMD_ALIVE },
	{ CMD_GET_FIRMWARE, CMD_ALIVE }
};

#define NUM_COMMANDS (sizeof(commandMap) / sizeof(commandMap[0]))

typedef struct {
	uint8_t indexW;
	uint8_t indexR;
	uint8_t buffer[PROTOCOL_BUFFER_SIZE];
	Byte_Flag_Struct flags;
	ProtocolFrame receivePck;
} ProtocolService;

#endif /* PROTOCOLTYPE_H_ */