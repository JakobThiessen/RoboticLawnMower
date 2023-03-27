/*
 * AVR128DB48_C_Example.c
 *
 * Created: 12/10/2020 1:11:14 PM
 * Author : JThiessen
 */ 

// light version:	-Wl,-u,vfprintf -lprintf_min		(~1.3KB)
// full version:	-Wl,-u,vfprintf -lprintf_flt -lm	(~2.0KB) (with floating point)
//#define F_CPU                           (4000000UL)         /* using default clock 4MHz*/

#define USE_FREE_RTOS

// ********************************************************************************
// Includes
// ********************************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <string.h>

#include "hal/i2c.h"
#include "hal/spi_basic.h"
#include "hal/port.h"
#include "hal/uart.h"
#include "hal/adc.h"

#include "commonDriver.h"

#ifdef USE_FREE_RTOS

	#include "OS/freeRTOS/include/FreeRTOS.h"
	#include "OS/freeRTOS/include/task.h"
	#include "OS/freeRTOS/include/stream_buffer.h"
	#include "OS/freeRTOS/include/message_buffer.h"
	
	#include "commonOS.h"
	#include "taskCommuncation.h"
	#include "taskEnvSensor.h"
	#include "taskGPS.h"
	#include "taskMotion.h"
#endif

// ********************************************************************************
// Macros and Defines
// ********************************************************************************

#define LED0	PIN3_bm
#define SW0		PIN2_bm
#define DIO		PIN7_bm

#define MOTOR_EN_M1	PIN3_bm
#define MOTOR_EN_M5	PIN2_bm

#define MAXPIX 8
#define COLORLENGTH MAXPIX/2
#define FADE 256/COLORLENGTH

#ifdef USE_FREE_RTOS
	#define COMMUNICATION_TASK_PRIORITY		1
	#define GPS_TASK_PRIORITY				3
	#define ENV_SESNOR_TASK_PRIORITY		4
	#define MOTION_TASK_PRIORITY			2
	
	void vControlTask(void* pvParameters);
	void vUartTask(void* pvParameters);
#endif
// ********************************************************************************
// Function Prototypes
// ********************************************************************************
void PORT_init(void);

// ********************************************************************************
// Global Variables
// ********************************************************************************

void PORT_init(void)
{
	/*
	PORTB.DIR |= LED0;
	PORTB.OUT |= LED0;

	PORTB.DIR &= ~(SW0);
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	PORTD.DIR |= DIO;
	PORTD.OUT |= DIO;
	*/
	PORTC.DIR |= MOTOR_EN_M1;
	PORTC.OUT |= MOTOR_EN_M1;
	
	PORTC.DIR |= MOTOR_EN_M5;
	PORTC.OUT |= MOTOR_EN_M5;
}


int main(void)
{
	// setup our stdio stream
//	stdout = &mystdout;

	CLOCK_OSCHF_crystal_PLL_init(CLKCTRL_FREQSEL_24M_gc);

	PORT_init();
	//TCA1_init();
	
	USART0_init(PIN4_bm, PIN5_bm, 1, 115200);
	
	I2C_0_init(PORTMUX_TWI0_DEFAULT_gc, I2C_SCL_FREQ);
	I2C_1_init(PORTMUX_TWI1_ALT2_gc, I2C_SCL_FREQ);	
				
	I2C_0_scan(0x01, 0x7F);
	I2C_1_scan(0x01, 0x7F);

/*		
	printf("I2C_0:												\n\r	\
			BMM150_I2C_ADDRESS_CSB_HIGH_SDO_HIGH		0x13	\n\r	\
			ADS1115_IIC_ADDRESS_0						0x48	\n\r	\
			ADS1115_IIC_ADDRESS_1						0x49	\n\r	\
			BMI160_I2C_ADDR_SEC							0x69	\n\r	\
			BME280_I2C_ADDR_SEC							0x77	\n\r	\
			I2C_1:												\n\r	\
			INA228										0x40	\n\r	\
			INA228										0x41	\n\r	\
			INA233										0x44	\n\r	\
			PCA9685_A0_A1								0x47	\n\r");
*/
	sei();

	PORTC_set_pin_level(MOTOR_EN_M1, true);
	PORTC_set_pin_level(MOTOR_EN_M5, true);
	
	static char tx_buff[256];
	sprintf((char*)tx_buff, "--> Test Test: started...\n\r");
	USART0_sendString(tx_buff);

	#ifdef USE_FREE_RTOS
		
		terminal_tx_buffer = xMessageBufferCreate(512);
		
		// Create task.
		xTaskCreate(vCommunicationTask,	"uart",		configMINIMAL_STACK_SIZE+1024,	NULL, COMMUNICATION_TASK_PRIORITY,	NULL);
		xTaskCreate(vGpsTask,			"gps",		configMINIMAL_STACK_SIZE+512,	NULL, GPS_TASK_PRIORITY,			NULL);
		xTaskCreate(vEnvSensorTask,		"sensor",	configMINIMAL_STACK_SIZE+512,	NULL, ENV_SESNOR_TASK_PRIORITY,		NULL);
		xTaskCreate(vMotionTask,		"motion",	configMINIMAL_STACK_SIZE+512,	NULL, MOTION_TASK_PRIORITY,			NULL);

		// Start scheduler.
		vTaskStartScheduler();

		return 0;
		
	#endif

}

/**************************************************************************//**
	* \fn static vApplicationIdleHook(void)
	*
	* \brief
	******************************************************************************/
void vApplicationIdleHook(void)
{

}
	
/**************************************************************************//**
	* \fn static vApplicationIdleHook(void)
	*
	* \brief
	******************************************************************************/
void vApplicationTickHook(void)
{

}
