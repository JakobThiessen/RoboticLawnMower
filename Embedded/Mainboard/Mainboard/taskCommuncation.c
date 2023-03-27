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

#include "taskCommuncation.h"

// ********************************************************************************
// Macros and Defines
// ********************************************************************************


// ********************************************************************************
// Variables
// ********************************************************************************
static char tx_buff[300];

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
		while (xMessageBufferReceive(terminal_tx_buffer, tx_buff, sizeof( tx_buff ), portMAX_DELAY) > 0)
		{
			USART0_sendString(tx_buff);
		}
	}
}
