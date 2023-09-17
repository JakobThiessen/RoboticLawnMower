/*
 * taskEnvSensor.c
 *
 * Created: 24.03.2023 18:56:49
 *  Author: jakob
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <string.h>

#include "OS/freeRTOS/include/FreeRTOS.h"
#include "OS/freeRTOS/include/task.h"
#include "OS/freeRTOS/include/stream_buffer.h"
#include "OS/freeRTOS/include/message_buffer.h"

#include "commonOS.h"
#include "commonDriver.h"
#include "commonDataMgmt.h"

#include "taskCommuncation.h"

// ********************************************************************************
// Macros and Defines
// ********************************************************************************


// ********************************************************************************
// Variables
// ********************************************************************************
static uint8_t tx_data[512];

/**************************************************************************//**
* \fn static void vUartTask(void* pvParameters)
*
* \brief
*
* \param[in]   pvParameters
******************************************************************************/

uint16_t rx_data_idx = 0;
char rx_data[512];

ISR(USART0_RXC_vect)
{
	cli();
	rx_data[rx_data_idx] = USART0.RXDATAL;
	rx_data_idx++;
	sei();
}

void vCommunicationTask(void* pvParameters)
{
	sprintf((char*)tx_data, "\n\r\n\r--> TASK vCommunicationTask: started...\n\r\n\r");
	USART0_sendString(tx_data);
	
	for(;;)
	{
		/*
		while (xMessageBufferReceive(terminal_tx_buffer, tx_buff, sizeof( tx_buff ), pdMS_TO_TICKS(20)) > 0)		// portMAX_DELAY
		{
			USART0_sendString(tx_buff);
		}
		*/
		tx_data[0] = 0x02;
		tx_data[1] = 0xFF;
		getGlbRoboterData(&tx_data[3], &tx_data[2]);
		
		USART0_sendBuffer(tx_data, tx_data[2] + 3);
		if (rx_data_idx > 10)
		{
			sprintf((char*)tx_data, "--> ComTask: receive [%02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X ...]\n\r", rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5], rx_data[6], rx_data[7]);
			USART0_sendString(tx_data);
			rx_data_idx = 0;
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
