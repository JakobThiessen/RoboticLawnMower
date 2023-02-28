/*
 * uart.c
 *
 * Created: 12/16/2020 2:58:25 PM
 *  Author: JThiessen
 */ 

#include "uart.h"

void USART3_init(void)
{
	PORTB.DIRSET = PIN0_bm;                             /* set pin 0 of PORT C (TXd) as output*/
	PORTB.DIRCLR = PIN1_bm;                             /* set pin 1 of PORT C (RXd) as input*/
	
	USART3.BAUD = (uint16_t)(USART3_BAUD_RATE(9600));   /* set the baud rate*/
	USART3.CTRLC = USART_CHSIZE0_bm | USART_CHSIZE1_bm;	/* set the data format to 8-bit*/
	USART3.CTRLB |= USART_TXEN_bm;                      /* enable transmitter*/
	//	USART3.CTRLB |= USART_RXEN_bm | USART_TXEN_bm; /* enable receiver and transmitter*/
}

void USART3_sendChar(char c)
{
	while(!(USART3.STATUS & USART_DREIF_bm))
	{
		;
	}
	
	USART3.TXDATAL = c;
}

int USART3_putchar_printf(char var, FILE *stream)
{
	USART3_sendChar(var);
	return 0;
}

void USART3_sendString(char *str)
{
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART3_sendChar(str[i]);
	}
}
