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

#define PROTOCOL_BUFFER_SIZE 64  // Buffer circular de 64 bytes
#define PROTOCOL_TOKEN ':'
#define PROTOCOL_PACING_TOKEN '-'
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
	CMD_RESPONSE_ALIVE           = 0x00, // Response: Alive confirmation
	CMD_RESPONSE_START           = 0x01, // Response for start command
	CMD_RESPONSE_STOP            = 0x02, // Response for stop command
	CMD_RESPONSE_SET_CONFIG      = 0x03, // Response for set configuration command
	CMD_RESPONSE_GET_CONFIG      = 0x04, // Response for get configuration command
	CMD_RESPONSE_CLEAR_STATS     = 0x05, // Response for clear statistics command
	CMD_RESPONSE_GET_STATS       = 0x06, // Response for get statistics command
	CMD_RESPONSE_GET_FIRMWARE    = 0x07, // Response for get firmware command
	CMD_RESPONSE_GET_REPOSITORY  = 0x08, // Response for get repository command
	CMD_RESPONSE_RESPONSE_ALIVE  = 0x09, // Response for alive check (response to an alive request)
	CMD_ALIVE                    = 0xA0, // Command indicating device is alive
	CMD_START                    = 0xB1, // Start command
	CMD_STOP                     = 0xB2, // Stop command
	CMD_SET_CONFIG               = 0xC0, // Command to set configuration
	CMD_GET_CONFIG               = 0xC1, // Command to get configuration
	CMD_GET_FIRMWARE             = 0xF0, // Command to get firmware
	CMD_GET_STATS                = 0xF1, // Command to get statistics
	CMD_CLEAR_STATS              = 0xF2, // Command to clear statistics
	CMD_GET_REPOSITORY           = 0xF3, // Command to get repository
	CMD_INVALID                  = 0xE0, // Invalid command (default error)
	// Add other commands as needed
} Command;


typedef struct {
	Command request;
	Command response;
} CommandMap;

typedef struct {
	volatile uint8_t indexW;
	volatile uint8_t indexR;
	volatile uint8_t buffer[PROTOCOL_BUFFER_SIZE];
	volatile Byte_Flag_Struct flags;
	volatile ProtocolFrame receivePck;
} ProtocolService;

#endif /* PROTOCOLTYPE_H_ */