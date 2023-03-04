/*
 * uart.c
 *
 * Created: 12/16/2020 2:58:25 PM
 *  Author: JThiessen
 */ 

#include "uart.h"

void USART0_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate)
{
	PORTA.DIRSET = txPin;                             /* set pin of PORT A (TXD) as output*/
	PORTA.DIRCLR = rxPin;                             /* set pin of PORT A (RXD) as input*/
	
	if (altPin != 0)
	{
		PORTMUX.USARTROUTEA |= 0x01;
	}

	USART0.BAUD = (uint16_t)(USART_BAUD_RATE(baudrate));   /* set the baud rate*/
	USART0.CTRLC = USART_CHSIZE0_bm | USART_CHSIZE1_bm;	/* set the data format to 8-bit*/
	USART0.CTRLB |= USART_TXEN_bm;                      /* enable transmitter*/
	//	USART0.CTRLB |= USART_RXEN_bm | USART_TXEN_bm; /* enable receiver and transmitter*/
}

void USART0_sendChar(char c)
{
	while(!(USART0.STATUS & USART_DREIF_bm))
	{
		;
	}
	
	USART0.TXDATAL = c;
}

int USART0_putchar_printf(char var, FILE *stream)
{
	USART0_sendChar(var);
	return 0;
}

void USART0_sendString(char *str)
{
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART0_sendChar(str[i]);
	}
}

/************************************************************************/
/*  UART 1                                                              */
/************************************************************************/

void USART1_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate)
{
	PORTC.DIRSET = txPin;                             /* set pin of PORT C (TXD) as output*/
	PORTC.DIRCLR = rxPin;                             /* set pin of PORT C (RXD) as input*/
	
	if (altPin != 0)
	{
		PORTMUX.USARTROUTEA |= 0x04;
	}
	
	USART1.BAUD = (uint16_t)(USART_BAUD_RATE(baudrate));   /* set the baud rate*/
	USART1.CTRLC = USART_CHSIZE0_bm | USART_CHSIZE1_bm;	/* set the data format to 8-bit*/
	USART1.CTRLB |= USART_TXEN_bm;                      /* enable transmitter*/
	//	USART1.CTRLB |= USART_RXEN_bm | USART_TXEN_bm; /* enable receiver and transmitter*/
}

void USART1_sendChar(char c)
{
	while(!(USART1.STATUS & USART_DREIF_bm))
	{
		;
	}
	
	USART1.TXDATAL = c;
}

int USART1_putchar_printf(char var, FILE *stream)
{
	USART1_sendChar(var);
	return 0;
}

void USART1_sendString(char *str)
{
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART1_sendChar(str[i]);
	}
}

/************************************************************************/
/*  UART 2                                                              */
/************************************************************************/

void USART2_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate)
{
	PORTF.DIRSET = txPin;                             /* set pin of PORT F (TXD) as output*/
	PORTF.DIRCLR = rxPin;                             /* set pin of PORT F (RXD) as input*/
	
	if (altPin != 0)
	{
		PORTMUX.USARTROUTEA |= 0x10;
	}
	
	USART2.BAUD = (uint16_t)(USART_BAUD_RATE(baudrate));   /* set the baud rate*/
	USART2.CTRLC = USART_CHSIZE0_bm | USART_CHSIZE1_bm;	/* set the data format to 8-bit*/
	USART2.CTRLB |= USART_TXEN_bm;                      /* enable transmitter*/
	//	USART2.CTRLB |= USART_RXEN_bm | USART_TXEN_bm; /* enable receiver and transmitter*/
}

void USART2_sendChar(char c)
{
	while(!(USART2.STATUS & USART_DREIF_bm))
	{
		;
	}
	
	USART2.TXDATAL = c;
}

int USART2_putchar_printf(char var, FILE *stream)
{
	USART2_sendChar(var);
	return 0;
}

void USART2_sendString(char *str)
{
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART2_sendChar(str[i]);
	}
}

/************************************************************************/
/*  UART 3                                                              */
/************************************************************************/

void USART3_init(uint8_t txPin, uint8_t rxPin, uint8_t altPin, uint32_t baudrate)
{
	PORTB.DIRSET = txPin;                             /* set pin of PORT B (TXD) as output*/
	PORTB.DIRCLR = rxPin;                             /* set pin of PORT B (RXD) as input*/
	
	if (altPin != 0)
	{
		PORTMUX.USARTROUTEA |= 0x40;
	}
	
	USART3.BAUD = (uint16_t)(USART_BAUD_RATE(baudrate));   /* set the baud rate*/
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
