/**
 * @file protocolType.h
 * @brief Define los tipos de datos relacionados al protocolo de comunicación serial.
 * 
 * Este protocolo utiliza una estructura de tramas con cabecera 'UNER', longitud,
 * token separador, comando, payload y checksum para verificación.
 * 
 * Define la máquina de estados del protocolo, los comandos disponibles y la estructura de servicio.
 *
 * @author kobac
 * @date 30-Mar-25
 */

/*
CMD_ALIVE: 55 4E 45 52 00 3A A0 96
CMD_START: 55 4E 45 52 01 3A B1 01 87
CMD_STOP: 55 4E 45 52 01 3A B2 00 85
CMD_SET_CONFIG: 55 4E 45 52 0B 3A C0 30 3A 43 2D 31 3A 42 2D 32 3A 41 B4 //PAYLOAD: 30 3A 43 2D 31 3A 42 2D 32 3A 41 ? equivale a la cadena "0:C-1:B-2:A"
CMD_GET_CONFIG: 55 4E 45 52 00 3A C1 F7
CMD_GET_FIRMWARE: 55 4E 45 52 00 3A F0 C6
CMD_GET_STATS: 55 4E 45 52 00 3A F1 C7
CMD_CLEAR_STATS: 55 4E 45 52 00 3A F2 C4
CMD_GET_REPOSITORY: 55 4E 45 52 00 3A F3 C5
CMD_CONFIG_RESET: 55 4E 45 52 00 3A F4 C2
CMD_INVALID: 55 4E 45 52 00 3A E0 D6
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
#define PROTOSERV_PROCESSING_PAYLOAD 8
#define PROTOSERV_VALIDATED 9
#define PROTOSERV_SEND 10
#define PROTOSERV_SEND_DONE 11

#define PROTOSERV_PROCESSING BIT0_MASK
#define PROTOSERV_CHECKDATA BIT1_MASK
#define PROTOSERV_RESET BIT2_MASK
#define PROTOSERV_CREATE_PCK BIT3_MASK

/**
 * @struct ProtocolFrame
 * @brief Representa una trama de comunicación del protocolo.
 *
 * Contiene la cabecera, longitud del payload, token, comando, puntero al payload y checksum.
 */
typedef struct {
	uint8_t header[4];           // HEADER (4 bytes)
	uint8_t length;              // LENGTH (1 byte)
	uint8_t token;               // TOKEN (1 byte)
	uint8_t cmd;                 // CMD (1 byte)
	uint8_t* payload;            // PAYLOAD (N bytes) //Marca la posicion en el buffer de protoService
	uint8_t checksum;            // CHECKSUM (1 byte)
} ProtocolFrame;

/**
 * @enum Command
 * @brief Enum que contiene todos los comandos del protocolo, incluyendo comandos y sus respuestas.
 */
typedef enum {
	// Responses
	CMD_RESPONSE_ALIVE           = 0x00,
	CMD_RESPONSE_START           = 0x01,
	CMD_RESPONSE_STOP            = 0x02,
	CMD_RESPONSE_SET_CONFIG      = 0x03,
	CMD_RESPONSE_GET_CONFIG      = 0x04,
	CMD_RESPONSE_CLEAR_STATS     = 0x05,
	CMD_RESPONSE_GET_STATS       = 0x06,
	CMD_RESPONSE_GET_FIRMWARE    = 0x07,
	CMD_RESPONSE_GET_REPOSITORY  = 0x08,
	CMD_RESPONSE_CONFIG_RESET    = 0x09,
	CMD_RESPONSE_DEBUG_ULTRA     = 0x0A,
	CMD_RESPONSE_DEBUG_SERVOS    = 0x0B,
	CMD_RESPONSE_DEBUG_SORTER    = 0x0C,
	CMD_RESPONSE_DEBUG_EEPROM    = 0x0D,
	CMD_RESPONSE_SET_IR_CONFIG   = 0x0E,
	CMD_RESPONSE_GET_IR_CONFIG	 = 0x0F,

	// Main commands
	CMD_ALIVE                    = 0xA0,
	CMD_START                    = 0xB1,
	CMD_STOP                     = 0xB2,
	CMD_SET_CONFIG               = 0xC0,
	CMD_GET_CONFIG               = 0xC1,
	CMD_SET_IR_CONFIG            = 0xC2,  // << NUEVO
	CMD_GET_IR_CONFIG            = 0xC3,  // << NUEVO

	CMD_GET_FIRMWARE             = 0xF0,
	CMD_GET_STATS                = 0xF1,
	CMD_CLEAR_STATS              = 0xF2,
	CMD_GET_REPOSITORY           = 0xF3,
	CMD_CONFIG_RESET             = 0xF4,

	// Debug commands (zona 0xE...)
	CMD_DEBUG_ULTRA              = 0xE1,
	CMD_DEBUG_SERVOS             = 0xE2,
	CMD_DEBUG_SORTER             = 0xE3,
	CMD_DEBUG_EEPROM             = 0xE4,

	// Fallback
	CMD_INVALID                  = 0xE0
} Command;

/**
 * @struct CommandMap
 * @brief Estructura para mapear comandos con sus respuestas correspondientes.
 */
typedef struct {
	Command request;
	Command response;
} CommandMap;

/**
 * @struct ProtocolService
 * @brief Estructura que maneja el estado del servicio de protocolo serial.
 *
 * Incluye buffer circular, índices de lectura/escritura, flags de control y la trama recibida.
 */
typedef struct {
	volatile uint8_t indexW;
	volatile uint8_t indexR;
	volatile uint8_t buffer[PROTOCOL_BUFFER_SIZE];
	volatile Byte_Flag_Struct flags;
	volatile ProtocolFrame receivePck;
} ProtocolService;

#endif /* PROTOCOLTYPE_H_ */