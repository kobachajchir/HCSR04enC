/*
 * USART_UTILS.h
 *
 * Created: 22-Mar-25 5:35:06 PM
 *  Author: kobac
 */ 


#ifndef USART_UTILS_H_
#define USART_UTILS_H_

#include <stdio.h>
#include <avr/io.h>


void USART_Init(uint16_t ubrr);
void USART_Transmit(unsigned char data);
int USART_putchar(char c, FILE *stream);
int USART_getchar(FILE *stream);

#endif /* USART_UTILS_H_ */