/*
 * uart.h
 *
 * Created: 12/16/2020 2:58:36 PM
 *  Author: JThiessen
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#define USART_BAUD_RATE(BAUD_RATE)     (64 * F_CPU / (16 * BAUD_RATE))

void USART0_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate);
void USART0_sendChar(char c);
void USART0_sendString(char *str);void USART0_sendBuffer(uint8_t *data, uint16_t size);int USART0_putchar_printf(char var, FILE *stream);

void USART1_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate);
void USART1_sendChar(char c);
void USART1_sendString(char *str);int USART1_putchar_printf(char var, FILE *stream);

void USART2_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate);
void USART2_sendChar(char c);
void USART2_sendString(char *str);int USART2_putchar_printf(char var, FILE *stream);

void USART3_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate);
void USART3_sendChar(char c);
void USART3_sendString(char *str);int USART3_putchar_printf(char var, FILE *stream);

//static FILE mystdout = FDEV_SETUP_STREAM(USART0_putchar_printf, NULL, _FDEV_SETUP_WRITE);

#endif /* UART_H_ */