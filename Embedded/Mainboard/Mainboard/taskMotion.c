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
#include <stdlib.h>
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
static int8_t setMotorParameter(uint8_t motorNr, uint16_t velocity, uint8_t direction, struct pca9685_dev *dev);

// ********************************************************************************
// Variables
// ********************************************************************************

struct ina228_dev	monitorMotor_0;
struct ina228_dev	monitorMotor_1;

struct pca9685_dev	pwmControl;

#define TASK_TIME			50				// ms
#define CONST_BACKWARD_CNT	10				//* TASK_TIME	// timeout
#define CONST_ROTATION_CNT	10				//* TASK_TIME	// timeout
#define CONST_STOP_CNT		2
#define CONST_ROTATION_DEG	45				// [Deg]
#define CONST_VELOCITY_HIGH 2000
#define CONST_VELOCITY_LOW	1000
#define CONST_MOWER_VEL		4000
#define CONST_DIST_LEVEL_1	1000			// [mm]
#define CONST_DIST_LEVEL_2	1000			// [mm]
#define CONST_DIST_LEVEL_3	800				// [mm]
#define CONST_DIST_LEVEL_4	400				// [mm]

enum stateMachine
{
	STATE_INIT,
	STATE_FORWARD,
	STATE_SLOW_FORWARD,
	STATE_BACKWARD,
	STATE_START_ROTATION,
	STATE_ROTATION,
	STATE_STOP,
	STATE_DISTANCE,
	STATE_WIRE,
	STATE_HOME,
	STATE_CHARGE,
	STATE_ERROR
};


/**************************************************************************//**
* \fn static void vUartTask(void* pvParameters)
*
* \brief
*
* \param[in]   pvParameters
******************************************************************************/
void vMotionTask(void* pvParameters)
{
	//static const char msg[] = "--> TASK vMotionTask: started...\n\r";
	//xMessageBufferSend(terminal_tx_buffer, msg, sizeof(msg), 0);

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
	//sprintf((char*)buffer, "--> TASK vMotionTask: TI INA228 [0] --> CHIP_ID: 0x%02X\n\r", monitorMotor_0.devID);
	//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

	ina228_init(&monitorMotor_1);
	//sprintf((char*)buffer, "--> TASK vMotionTask: TI INA228 [1] --> CHIP_ID: 0x%02X\n\r", monitorMotor_1.devID);
	//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

	pca9685_init(&pwmControl);
	//sprintf((char*)buffer, "--> TASK vMotionTask: PCA9685 INIT --> CHIP_ID: 0x%02X\n\r", pwmControl.chip_id);
	//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

	uint8_t collosionStatus = 0;
	uint16_t velocity_1 = 0;
	uint16_t velocity_2 = 0;
	uint16_t dir_1 = 0;
	uint16_t dir_2 = 0;
	uint16_t mowerMotorVel = 0;
	float voltage_0, voltage_1;
	float current_0, current_1;
	uint16_t counterCollosion = 0;
	uint16_t state = STATE_INIT;
	uint16_t state_old = STATE_INIT;
	
	uint16_t backward_time = 0;
	uint16_t rotation_time = 0;
	uint16_t stop_time = 0;
	
	int16_t t1 = 0;
	int16_t t2 = 0;
	int16_t d = 0;
	
	for ( ;; )
	{	
		state_old = state;
		ina228_voltage(&voltage_0, &monitorMotor_0);
		ina228_voltage(&voltage_1, &monitorMotor_1);
		ina228_current(&current_0, &monitorMotor_0);
		ina228_current(&current_1, &monitorMotor_1);

		glbRoboterData.motorCurrent[0] = (int16_t)(current_0 * 1);
		glbRoboterData.motorVoltage[0] = (int16_t)(voltage_0 * 1000);
		glbRoboterData.motorCurrent[1] = (int16_t)(current_1 * 1);
		glbRoboterData.motorVoltage[1] = (int16_t)(voltage_1 * 1000);

		glbRoboterData.sens_collosion_00 = PORTD_get_pin_level(SENSOR_COLLISION_00);
		glbRoboterData.sens_collosion_01 = PORTD_get_pin_level(SENSOR_COLLISION_01);
		glbRoboterData.sens_collosion_02 = PORTD_get_pin_level(SENSOR_COLLISION_02);
/*		
		if (glbRoboterData.errorNeigung == 1)
		{
			state = STATE_ERROR;
		}
*/

		switch (state)
		{
			case STATE_INIT:
				velocity_1 = 0;
				velocity_2 = 0;
				mowerMotorVel = 0;
				state = STATE_DISTANCE;
				break;
			
			case STATE_FORWARD:
				velocity_1 = CONST_VELOCITY_HIGH;
				velocity_2 = CONST_VELOCITY_HIGH;
				dir_1 = 1;
				dir_2 = 0;
				mowerMotorVel = CONST_MOWER_VEL;
				state = STATE_DISTANCE;
				break;
			
			case STATE_SLOW_FORWARD:
				velocity_1 = CONST_VELOCITY_LOW;
				velocity_2 = CONST_VELOCITY_LOW;
				mowerMotorVel = CONST_MOWER_VEL;
				dir_1 = 1;
				dir_2 = 0;
				state = STATE_DISTANCE;
				break;
				
			case STATE_BACKWARD:
				velocity_1 = CONST_VELOCITY_HIGH;
				velocity_2 = CONST_VELOCITY_HIGH;
				dir_1 = 0;
				dir_2 = 1;
				mowerMotorVel = CONST_MOWER_VEL;
				if (backward_time < CONST_BACKWARD_CNT)
				{
					state = STATE_BACKWARD;
					backward_time++;
				} 
				else
				{
					state = STATE_START_ROTATION;
					backward_time = 0;
				}
				break;
			
			case STATE_START_ROTATION:
			
				rotation_time = 0;
				t1 = (int16_t)(glbRoboterData.CompassDeg / 1000);
				t2 = 0;
				d = 0;
				state = STATE_ROTATION;
				
				break;
			
			case STATE_ROTATION:
				velocity_1 = 0;
				velocity_2 = CONST_VELOCITY_LOW;
				dir_1 = 1;
				dir_2 = 0;
				mowerMotorVel = CONST_MOWER_VEL;
				
				t2 = (int16_t)(glbRoboterData.CompassDeg / 1000);
				d = (abs(t1 - t2) % 360);
				if (d > 180)    d = 360 - d;

				if ( (rotation_time < CONST_ROTATION_CNT) && ( d < CONST_ROTATION_DEG) )
				{
					state = STATE_ROTATION;
					rotation_time++;
				}
				else
				{
					state = STATE_DISTANCE;
					rotation_time = 0;
				}
				
				break;
			
			case STATE_STOP:
				//velocity_1 = 0;
				//velocity_2 = 0;
				//mowerMotorVel = 0;
				
				stop_time++;
				if ( (collosionStatus != 0) && (stop_time >= CONST_STOP_CNT) )
				{
					state = STATE_BACKWARD;
					stop_time = 0;
				} 
				else
				{
					if (stop_time >= CONST_STOP_CNT)
					{
						state = STATE_DISTANCE;
						stop_time = 0;
					}
					
				}
				
				break;
				
			case STATE_DISTANCE:
			
				if (glbRoboterData.sens_collosion_02 == 0)
				{
					state = STATE_ERROR;
				}
				else
				{
					if ( (glbRoboterData.sens_collosion_00 == 0) || (glbRoboterData.sens_collosion_01 == 0) )
					{
						state = STATE_STOP;
						collosionStatus = 1;
					}
					else
					{
						collosionStatus = 0;

						if(glbRoboterData.sens_dist_00 >= CONST_DIST_LEVEL_1)
						{
							state = STATE_FORWARD;
						}
						else if (glbRoboterData.sens_dist_00 < CONST_DIST_LEVEL_2)
						{
							state = STATE_SLOW_FORWARD;
						
							if(glbRoboterData.sens_dist_00 <= CONST_DIST_LEVEL_3)
							{
								if (glbRoboterData.sens_dist_00 > CONST_DIST_LEVEL_4)
								{
									state = STATE_START_ROTATION;
								}
								else
								{
									counterCollosion++;
									if (counterCollosion > 5)
									{
										state = STATE_BACKWARD;
										counterCollosion = 0;
									}
									else
									{
										state = STATE_STOP;
									
									}
								}
							}
						}
					}

				}
				break;
					
			case STATE_HOME:
				break;
			
			case STATE_CHARGE:
				break;
				
			case STATE_ERROR:
				if (glbRoboterData.sens_collosion_02 != 0)
				{
					state = STATE_DISTANCE;
				}

				break;
				
			default:
				velocity_1 = 0;
				velocity_2 = 0;
				dir_1 = 1;
				dir_2 = 0;
				mowerMotorVel = 0;
				break;			
		}

		glbRoboterData.state_MotionStateMachine = state;
		
		if( (state == STATE_STOP) || (state == STATE_ERROR) )
		{
			PORTC_set_pin_level(MOTOR_EN_M1, true);
			PORTC_set_pin_level(MOTOR_EN_M5, true);
			
							velocity_1 = 0;
							velocity_2 = 0;
							dir_1 = 1;
							dir_2 = 0;
							mowerMotorVel = 0;
		}
		else
		{
			PORTC_set_pin_level(MOTOR_EN_M1, false);
			PORTC_set_pin_level(MOTOR_EN_M5, false);
			
			if (state_old != state)
			{
				setMotorParameter(1, velocity_1,	dir_1, &pwmControl);
				setMotorParameter(2, velocity_2,	dir_2, &pwmControl);
				setMotorParameter(5, mowerMotorVel,	1, &pwmControl);
			}

		}
		
		setMotorParameter(1, velocity_1,	dir_1, &pwmControl);
		setMotorParameter(2, velocity_2,	dir_2, &pwmControl);
		setMotorParameter(5, mowerMotorVel,	1, &pwmControl);
		
		vTaskDelay(pdMS_TO_TICKS(TASK_TIME));
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
static int8_t setMotorParameter(uint8_t motorNr, uint16_t velocity, uint8_t direction, struct pca9685_dev *dev)
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