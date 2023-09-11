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

#include "driver/INA228/ina228.h"
#include "driver/PCA9685/pca9685.h"

#include "taskMotion.h"


// ********************************************************************************
// Macros and Defines
// ********************************************************************************


// ********************************************************************************
// Finctions
// ********************************************************************************
int8_t setMotorParameter(uint8_t motorNr, uint16_t velocity, uint8_t direction, struct pca9685_dev *dev);

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
		float current_0, current_1;

		ina228_voltage(&voltage_0, &monitorMotor_0);
		ina228_voltage(&voltage_1, &monitorMotor_1);
		ina228_current(&current_0, &monitorMotor_0);
		ina228_current(&current_1, &monitorMotor_1);
		
		setMotorParameter(1, 2048, 1, &pwmControl);
		setMotorParameter(2, 2048, 1, &pwmControl);
		
		setMotorParameter(5, 4000, 1, &pwmControl);
		glbRoboterData.motorCurrent[0] = (int16_t)(current_0 * 1);
		glbRoboterData.motorVoltage[0] = (int16_t)(voltage_0 * 1000);
		glbRoboterData.motorCurrent[1] = (int16_t)(current_1 * 1);
		glbRoboterData.motorVoltage[1] = (int16_t)(voltage_1 * 1000);
		
		//sprintf((char*)buffer, "--> vMotionTask: M[0] U= %.03fmV I= %.03fmA\tM[1] U= %.03fmV I= %.03fmA\n\r", voltage_0, current_0, voltage_1, current_1);
		//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
		glbRoboterData.sens_collosion_00 = PORTD_get_pin_level(SENSOR_COLLISION_00);
		glbRoboterData.sens_collosion_01 = PORTD_get_pin_level(SENSOR_COLLISION_01);
		glbRoboterData.sens_collosion_02 = PORTD_get_pin_level(SENSOR_COLLISION_02);
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}


/*
PCA9685 Channels list
0	MOTOR 1		DIR 1 = Z; 0 = F
1	MOTOR 1		VEL
2	MOTOR 2		DIR 1 = Z; 0 = F
3	MOTOR 2		VEL
4	MOTOR 3		DIR 1 = Z; 0 = F
5	MOTOR 3		VEL
6	MOTOR 4		DIR 1 = Z; 0 = F
7	MOTOR 4		VEL
15	MOTOR 5		DIR 1 = Z; 0 = F
14	MOTOR 5		VEL
	
*/
int8_t setMotorParameter(uint8_t motorNr, uint16_t velocity, uint8_t direction, struct pca9685_dev *dev)
{
	uint16_t on = 0;
	uint16_t off = 0;
	
	if (velocity > 4095)
	{
		velocity = 4095;
	}
	
	switch (motorNr)
	{
		case 1:
			if (direction)
			{
				on = 4096;
				off = 0;
			}
			else
			{
				on = 0;
				off = 4096;
			}
			pca9685_setPWM(0, on, off, dev);
			
			pca9685_setPWM(1, 0, velocity, dev);
			break;
			
		case 2:
			if (direction)
			{
				on = 4096;
				off = 0;
			}
			else
			{
				on = 0;
				off = 4096;
			}
			pca9685_setPWM(2, on, off, dev);
			
			pca9685_setPWM(3, 0, velocity, dev);
			break;
		
		case 5:
			if (direction)
			{
				on = 4096;
				off = 0;
			}
			else
			{
				on = 0;
				off = 4096;
			}
			pca9685_setPWM(15, on, off, dev);
			
			pca9685_setPWM(14, 0, velocity, dev);
			break;	
		
		default:
			break;
	}
	
	return 0;
}