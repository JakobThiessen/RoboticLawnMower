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
#include <stdbool.h>

#include "OS/freeRTOS/include/FreeRTOS.h"
#include "OS/freeRTOS/include/task.h"
#include "OS/freeRTOS/include/stream_buffer.h"
#include "OS/freeRTOS/include/message_buffer.h"

#include "commonOS.h"
#include "commonDriver.h"

#include "driver/lwgps/lwgps.h"

#include "taskGPS.h"

// ********************************************************************************
// Macros and Defines
// ********************************************************************************

#define GPS_DEBUG 1
//#define GPS_DEBUG 0

#if !LWGPS_CFG_STATUS
//#error "this test must be compiled with -DLWGPS_CFG_STATUS=1"
#endif /* !LWGPS_CFG_STATUS */

/* GPS handle */
lwgps_t hgps;

// ********************************************************************************
// Variables
// ********************************************************************************

/**
 * \brief           Dummy data from GPS receiver
 */
const char gps_rx_data_test[] = ""
                "$GPRMC,183729,A,3907.356,N,12102.482,W,000.0,360.0,080301,015.5,E*6F\r\n"
                "$GPRMB,A,,,,,,,,,,,,V*71\r\n"
                "$GPGGA,183730,3907.356,N,12102.482,W,1,05,1.6,646.4,M,-24.1,M,,*75\r\n"
                "$GPGSA,A,3,02,,,07,,09,24,26,,,,,1.6,1.6,1.0*3D\r\n"
                "$GPGSV,2,1,08,02,43,088,38,04,42,145,00,05,11,291,00,07,60,043,35*71\r\n"
                "$GPGSV,2,2,08,08,02,145,00,09,46,303,47,24,16,178,32,26,18,231,43*77\r\n"
                "$PGRME,22.0,M,52.9,M,51.0,M*14\r\n"
                "$GPGLL,3907.360,N,12102.481,W,183730,A*33\r\n"
                "$PGRMZ,2062,f,3*2D\r\n"
                "$PGRMM,WGS84*06\r\n"
                "$GPBOD,,T,,M,,*47\r\n"
                "$GPRTE,1,1,c,0*07\r\n"
                "$GPRMC,183731,A,3907.482,N,12102.436,W,000.0,360.0,080301,015.5,E*67\r\n"
                "$GPRMB,A,,,,,,,,,,,,V*71\r\n";

uint16_t gps_rx_data_idx = 0;
char gps_rx_data[2048];

ISR(USART3_RXC_vect)
{
	cli();
	gps_rx_data[gps_rx_data_idx] = USART3.RXDATAL;
	gps_rx_data_idx++;
	sei();
}

const lwgps_statement_t expected[] = {
	STAT_RMC,
	STAT_UNKNOWN,
	STAT_GGA,
	STAT_GSA,
	STAT_GSV,
	STAT_GSV,
	STAT_UNKNOWN,
	STAT_UNKNOWN,
	STAT_UNKNOWN,
	STAT_CHECKSUM_FAIL,
	STAT_UNKNOWN,
	STAT_UNKNOWN,
	STAT_RMC,
	STAT_UNKNOWN
};

static int err_cnt;

void callback(lwgps_statement_t res)
{
	static int i;

	if (res != expected[i])
	{
		printf("failed i %d, expected res %d but received %d\n",
		i, expected[i], res);
		++err_cnt;
	}

	++i;
}

uint8_t buffer[300];
void vGpsTask(void* pvParameters)
{
	static const char msg[] = "--> TASK vGpsTask: started...\n\r";
	xMessageBufferSend(terminal_tx_buffer, msg, sizeof(msg), 0);

	
	
	/* Init GPS */
	bool status = lwgps_init(&hgps);
	sprintf((char*)buffer, "--> TASK vGpsTask: lwgps init:	%d\n\r", (int8_t)status);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
	
	for ( ;; )
	{
		/* Process all input data */
		//lwgps_process(&hgps, gps_rx_data, strlen(gps_rx_data), callback);
		uint8_t gpsResult = lwgps_process(&hgps, gps_rx_data, strlen(gps_rx_data) );

		if(gpsResult == 1 && GPS_DEBUG == 0)
		{
			sprintf((char*)buffer, "--> TASK vGpsTask: LONG %06f, LAT %06f, GPS TIME %02d:%02d:%02d; size %d\r\n", hgps.longitude, hgps.latitude, hgps.hours, hgps.minutes, hgps.seconds, gps_rx_data_idx);
			xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
			memset(gps_rx_data, 0, sizeof(gps_rx_data));
			gps_rx_data_idx = 0;
		}
		else
		{
			char delimiter[] = "$";
			char *ptr;

			ptr = strtok(gps_rx_data, delimiter);

			while(ptr != NULL)
			{
				sprintf((char*)buffer, "--> TASK vGpsTask: %s\r\n", ptr);
				xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
				// naechsten Abschnitt erstellen
				ptr = strtok(NULL, delimiter);
			}
			memset(gps_rx_data, 0, sizeof(gps_rx_data));
			gps_rx_data_idx = 0;
		}
		
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}