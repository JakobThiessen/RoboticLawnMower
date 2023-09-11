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
static char tx_buff[300];
static uint8_t tx_data[512];

/**************************************************************************//**
* \fn static void vUartTask(void* pvParameters)
*
* \brief
*
* \param[in]   pvParameters
******************************************************************************/
void vCommunicationTask(void* pvParameters)
{
	sprintf((char*)tx_buff, "--> TASK vCommunicationTask: started...\n\r");
	USART0_sendString(tx_buff);
	
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
		vTaskDelay(pdMS_TO_TICKS(20));
	}
}
