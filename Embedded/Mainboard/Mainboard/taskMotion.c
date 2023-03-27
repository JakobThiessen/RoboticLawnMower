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

#include "driver/INA228/ina228.h"
#include "driver/PCA9685/pca9685.h"

#include "taskMotion.h"


// ********************************************************************************
// Macros and Defines
// ********************************************************************************


// ********************************************************************************
// Variables
// ********************************************************************************

struct ina228_dev	monitorMotor_0;
struct ina228_dev	monitorMotor_1;

struct pca9685_dev	pwmControl;
uint8_t buffer[300];

/**************************************************************************//**
* \fn static void vUartTask(void* pvParameters)
*
* \brief
*
* \param[in]   pvParameters
******************************************************************************/
void vMotionTask(void* pvParameters)
{
	static const char msg[] = "--> TASK vMotionTask: started...\n\r";
	xMessageBufferSend(terminal_tx_buffer, msg, sizeof(msg), 0);

	uint8_t dev_addr_monitroMotor_0 = INA228_SLAVE_ADDRESS;
	monitorMotor_0.intf_ptr = &dev_addr_monitroMotor_0;
	monitorMotor_0.shunt_ADCRange = 0;
	monitorMotor_0.read = user_i2c_1_read;
	monitorMotor_0.write = user_i2c_1_write;
	monitorMotor_0.delay_us = user_delay_us;
	
	uint8_t dev_addr_monitroMotor_1 = INA228_SLAVE_ADDRESS + 1;
	monitorMotor_1.intf_ptr = &dev_addr_monitroMotor_1;
	monitorMotor_1.shunt_ADCRange = 0;
	monitorMotor_1.read = user_i2c_1_read;
	monitorMotor_1.write = user_i2c_1_write;
	monitorMotor_1.delay_us = user_delay_us;

	pwmControl.i2c_addr = PCA9685_I2C_ADDRESS(0, 0, 0, 1, 1, 1);
	pwmControl.read = user_i2c_1_read_pca;
	pwmControl.write = user_i2c_1_write_pca;
	pwmControl.delay_ms = user_delay_ms;

	ina228_init(&monitorMotor_0);
	sprintf((char*)buffer, "--> TASK vMotionTask: TI INA228 [0] --> CHIP_ID: 0x%02X\n\r", monitorMotor_0.devID);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

	ina228_init(&monitorMotor_1);
	sprintf((char*)buffer, "--> TASK vMotionTask: TI INA228 [1] --> CHIP_ID: 0x%02X\n\r", monitorMotor_1.devID);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

	pca9685_init(&pwmControl);
	sprintf((char*)buffer, "--> TASK vMotionTask: PCA9685 INIT --> CHIP_ID: 0x%02X\n\r", pwmControl.chip_id);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

	for ( ;; )
	{		
		float voltage_0, voltage_1;
//		float dieTemp_0, dieTemp_1;
		ina228_voltage(&voltage_0, &monitorMotor_0);
		ina228_voltage(&voltage_1, &monitorMotor_1);
//		ina228_dietemp(&dieTemp_0, &monitorMotor_0);
//		ina228_dietemp(&dieTemp_1, &monitorMotor_1);
		
		for (int i = 0; i < 16; i++)
		{
			pca9685_setPWM(i, (200 + i * 10), (1048 + i * 10), &pwmControl);
//			pca9685_getPWM(i, &on, &off, &pwmControl);
		}
		
		sprintf((char*)buffer, "--> vMotionTask: M[0] U= %.03fmV\tM[1] U= %.03fmV\n\r", voltage_0, voltage_1);
		xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
		
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
