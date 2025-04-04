/*
 * protocol_utils.h
 *
 * Created: 30-Mar-25 7:50:20 PM
 *  Author: kobac
 */ 


#ifndef PROTOCOL_UTILS_H_
#define PROTOCOL_UTILS_H_

#include "../../main.h"
#include "../../types/protocolTypes.h"

void initProtocolService(ProtocolService* service);
uint8_t calculate_checksum(ProtocolFrame* pak);
bool verify_header();
bool process_protocol_buffer();
void clear_receive_pck();
uint8_t calculatePayload();
void createPck(uint8_t cmd, uint8_t* payload, uint8_t payloadLength);
bool validatePck();
uint8_t create_payload(Command cmd);
uint8_t protocolTask();
void doAction(uint8_t cmd);

#endif /* PROTOCOL_UTILS_H_ */