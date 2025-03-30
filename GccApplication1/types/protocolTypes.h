/*
 * protocolType.h
 *
 * Created: 30-Mar-25 7:47:19 PM
 *  Author: kobac
 */ 


#ifndef PROTOCOLTYPE_H_
#define PROTOCOLTYPE_H_

#include "bitmapType.h"

#define PROTOCOL_BUFFER_SIZE 32

#define PROTOSERV_ BIT0_MASK;

#define PROTOSERV_IDLE BIT0_MASK;
#define PROTOSERV_READING_HEADER BIT1_MASK;
#define PROTOSERV_READING_LEN BIT2_MASK; //encuentra : y pasa a reading cmd
#define PROTOSERV_READING_CMD BIT3_MASK;
#define PROTOSERV_READING_PAYLOAD BIT4_MASK;
#define PROTOSERV_READING_CHK BIT5_MASK;
#define PROTOSERV_CALCULATING_CHK BIT6_MASK;
#define PROTOSERV_DONE BIT7_MASK;

typedef struct {
	uint8_t header[4];           // HEADER (4 bytes)
	uint8_t length;              // LENGTH (1 byte)
	uint8_t token;               // TOKEN (1 byte)
	uint8_t cmd;                 // CMD (1 byte)
	uint8_t* payload;            // PAYLOAD (N bytes)
	uint8_t checksum;            // CHECKSUM (1 byte)
} ProtocolFrame;

typedef enum {
	CMD_START = 0xA0,           // Comando de inicio
	CMD_STOP = 0xA1,            // Comando para detener alguna operación
	CMD_SET_CONFIG = 0xC0,          // Comando de configuración - setear
	CMD_GET_CONFIG = 0xC1,          // Comando de configuración - obtener
	CMD_GET_FIRMWARE = 0xF0,          // Comando de configuración - obtener
	// Agrega otros comandos según necesidad
} Command;

typedef struct {
	uint8_t indexW;
	uint8_t indexR;
	uint8_t buffer[PROTOCOL_BUFFER_SIZE];
	Byte_Flag_Struct flags;
} ProtocolService;

#endif /* PROTOCOLTYPE_H_ */