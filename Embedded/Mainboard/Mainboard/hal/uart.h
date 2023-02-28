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

#define USART3_BAUD_RATE(BAUD_RATE)     ((float)(64 * F_CPU / (16 * (float)BAUD_RATE)) + 0.5)

void USART3_init(void);
void USART3_sendChar(char c);
void USART3_sendString(char *str);int USART3_putchar_printf(char var, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(USART3_putchar_printf, NULL, _FDEV_SETUP_WRITE);

#endif /* UART_H_ */