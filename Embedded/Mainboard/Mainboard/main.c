/*
 * AVR128DB48_C_Example.c
 *
 * Created: 12/10/2020 1:11:14 PM
 * Author : JThiessen
 */ 

// light version:	-Wl,-u,vfprintf -lprintf_min		(~1.3KB)
// full version:	-Wl,-u,vfprintf -lprintf_flt -lm	(~2.0KB) (with floating point)
//#define F_CPU                           (4000000UL)         /* using default clock 4MHz*/

//#define USE_FREE_RTOS

// ********************************************************************************
// Includes
// ********************************************************************************
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/cpufunc.h>

#ifdef USE_FREE_RTOS
	#include "OS/freeRTOS/include/FreeRTOS.h"
	#include "OS/freeRTOS/include/task.h"
#endif

#include "hal/port.h"
#include "hal/uart.h"
#include "hal/adc.h"
#include "hal/i2c.h"
#include "hal/spi_basic.h"
//#include "hal/timer.h"
#include "hal/clock.h"

#include "driver/BME280/bme280_defs.h"
#include "driver/BMI160/bmi160_defs.h"
#include "driver/BMM150/bmm150_defs.h"
#include "driver/INA228/INA228.h"


// ********************************************************************************
// Macros and Defines
// ********************************************************************************
#define LED0	PIN3_bm
#define SW0		PIN2_bm
#define DIO		PIN7_bm

#define MAXPIX 8
#define COLORLENGTH MAXPIX/2
#define FADE 256/COLORLENGTH

#ifdef USE_FREE_RTOS
	#define mainLED_TASK_PRIORITY		(tskIDLE_PRIORITY )
	#define mainUART_TASK_PRIORITY		1
	#define mainSESNOR_TASK_PRIORITY	2
	#define mainPWM_TASK_PRIORITY		2
#endif
// ********************************************************************************
// Function Prototypes
// ********************************************************************************
void PORT_init(void);
void BUTTON_releaseCallback(void);
void BUTTON_pressCallback(void);
void toggleDIO(uint16_t count);

void drawOuterFrame(void);
void drawTemeprature(int32_t t);
void drawAkku(uint8_t stat);

#ifdef USE_FREE_RTOS
	static void vBlinkLed(void* pvParameters);
	static void vUartTask(void* pvParameters);
	static void vPwmTask(void* pvParameters);
	static void vSensorTask(void* pvParameters);
#endif
// ********************************************************************************
// Global Variables
// ********************************************************************************

void PORT_init(void)
{
	PORTB.DIR |= LED0;
	PORTB.OUT |= LED0;

	PORTB.DIR &= ~(SW0);
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	
	PORTD.DIR |= DIO;
	PORTD.OUT |= DIO;
}

void BUTTON_releaseCallback(void)
{
	PORTB.OUTSET |= LED0;
}

void BUTTON_pressCallback(void)
{
	PORTB.OUTCLR |= LED0;
}

int8_t user_i2c_read_HDC1080(uint8_t reg_addr, uint8_t *data, uint32_t len, uint16_t devAddr)
{
	int8_t rslt = 0;
	
	rslt = I2C_0_sendData(devAddr, &reg_addr, 1);
	_delay_ms(20);
	rslt += I2C_0_getData(devAddr, data, len);
	
	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, uint8_t *data, uint32_t len, uint16_t devAddr)
{
	uint8_t tmp [20];
	int8_t rslt = 0;
	
	tmp[0] = reg_addr;
	memcpy(&tmp[1], data, len);
	
	rslt = I2C_0_sendData(devAddr, tmp, len+1);
    return rslt;
}

int8_t user_i2c_write_ssd1306(uint8_t dev_addr, uint8_t *data, uint16_t len, bool stop);
int8_t user_i2c_write_ssd1306(uint8_t dev_addr, uint8_t *data, uint16_t len, bool stop)
{
	int8_t rslt = 0;
	rslt = I2C_0_sendData(dev_addr, data, len);
	
 	if(stop == true)
 	{
 		//i2c_master_send_stop(&i2c_master_instance);
 	}
	    
	return rslt;
}


/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period_us, void *intf_ptr)
{
	for(int p = 0; p < period_us; p++)
		_delay_us(1);
}

void user_delay_ms(uint32_t period_ms)
{
	for(int p = 0; p < period_ms; p++)
		_delay_ms(1);
}

void configuration_spi(void)
{
	// PORTA PIN 4	MOSI
	// PORTA PIN 5	MISO
	// PORTA PIN 6	SCLK
	// PORTA PIN 7	/CS
	
	// SCLK - Set pin direction to output
	PORTA_set_pin_dir(PIN7_bp, PORT_DIR_OUT);
	// Initial level
	PORTA_set_pin_level(PIN7_bp, true);
		
	SPI_MASTER_initialization();
}

uint8_t write_spi_data(uint8_t data)
{
	uint8_t ret = 0;
	PORTA_set_pin_level(PIN7_bp, false);
	ret = SPI_MASTER_exchange_byte(data);
	PORTA_set_pin_level(PIN7_bp, true);
	
	return ret;
}

uint8_t write_spi_buffer(uint8_t *buffer, uint8_t len)
{
	uint8_t ret = 0;
	PORTA_set_pin_level(PIN7_bp, false);
	SPI_MASTER_exchange_block(buffer, len);
	_delay_us(2);
	PORTA_set_pin_level(PIN7_bp, true);
	
	return ret;
}

int main(void)
{
	// setup our stdio stream
	stdout = &mystdout;
	CLOCK_OSCHF_crystal_PLL_16M_init();
	
	PORT_init();
	//TCA1_init();
	
	USART3_init();
	
	VREF0_init();
	ADC0_init();
	ADC0_start();
	
	I2C_0_init();
	
	configuration_spi();
	
	sei();
/*	
	dev_Sensor.dev_address = HDC1080_IIC_ADDRESS;
	dev_Sensor.settings = 0x3000;
	dev_Sensor.read = user_i2c_read_HDC1080;
	dev_Sensor.write = user_i2c_write;
	dev_Sensor.delay_us = user_delay_us;
*/	
	uint16_t ManufacID = 0;
	uint16_t DeviceId = 0;
	
/*	printf("\n***************************************************\n\r");
	hdc1080_init(&dev_Sensor);
	hdc1080_readManufacturerId(&ManufacID, &dev_Sensor);
	hdc1080_readDeviceId(&DeviceId, &dev_Sensor);
	printf("HDC Manufacturer ID:	%04X\n\r", ManufacID);
	printf("HDC Device ID:			%04X\n\r", DeviceId);
	printf("***************************************************\n\r");
*/

	#ifdef USE_FREE_RTOS
		// Create task.
		xTaskCreate(vBlinkLed,	"blink",	configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY,		NULL);
		xTaskCreate(vUartTask,	"uart",		configMINIMAL_STACK_SIZE, NULL, mainUART_TASK_PRIORITY,		NULL);
		xTaskCreate(vSensorTask,"sensor",	configMINIMAL_STACK_SIZE, NULL, mainSESNOR_TASK_PRIORITY,	NULL);
		xTaskCreate(vPwmTask,	"pwm",		configMINIMAL_STACK_SIZE, NULL, mainPWM_TASK_PRIORITY,		NULL);

		// Start scheduler.
		vTaskStartScheduler();

		return 0;
	
	#else
	
		int16_t temp_C;
		uint16_t adcVal;
		float hdc1080_Temp = 0;
		float hdc1080_Humidity = 0;
		uint16_t dutyCycle = 0;
	
		while (1)
		{
			//if (PORTB.IN & SW0)
			//{
				//BUTTON_releaseCallback();
			//}

			/*
			BUTTON_pressCallback();
			_delay_ms(50);
			BUTTON_releaseCallback();
			_delay_ms(50);
			*/

			adcVal = ADC0_read();
			temp_C = temperatureConvert(adcVal);
		
			//USART3_sendString("Hello World!\n\r");
			printf("Temp (ADC): %d HDC1080 T: %.02f H: %.02f\n\r", temp_C, hdc1080_Temp, hdc1080_Humidity);
		
			I2C_0_scan(0x00, 0x7F);
		
		}
		
	#endif
}

#ifdef USE_FREE_RTOS
	/**************************************************************************//**
	* \fn static vApplicationIdleHook(void)
	*
	* \brief
	******************************************************************************/
	void vApplicationIdleHook(void)
	{
		printf("--> TASK: IDLE\n\r");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}

	/******************************************************************************
	* Private function definitions.
	******************************************************************************/

	/**************************************************************************//**
	* \fn static void vBlinkLed(void* pvParameters)
	*
	* \brief
	*
	* \param[in]   pvParameters
	******************************************************************************/
	static void vBlinkLed(void* pvParameters)
	{
		//printf("--> TASK vBlinkLed: init\n\r");

		while(1)
		{
			BUTTON_pressCallback();
			vTaskDelay(pdMS_TO_TICKS(10));
			BUTTON_releaseCallback();
			vTaskDelay(pdMS_TO_TICKS(10));
		}
	}

	/**************************************************************************//**
	* \fn static void vUartTask(void* pvParameters)
	*
	* \brief
	*
	* \param[in]   pvParameters
	******************************************************************************/
	static void vUartTask(void* pvParameters)
	{
		//printf("--> TASK vUartTask: init\n\r");
		uint16_t i = 0;

		while(1)
		{
			printf("--> TASK vUartTask: %d\n\r", i++);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
	
	/**************************************************************************//**
	* \fn static void vUartTask(void* pvParameters)
	*
	* \brief
	*
	* \param[in]   pvParameters
	******************************************************************************/
	static void vSensorTask(void* pvParameters)
	{
		//printf("--> TASK vSensorTask: init\n\r");

		int16_t temp_C;
		uint16_t adcVal;
		float hdc1080_Temp = 0;
		float hdc1080_Humidity = 0;
		
		for ( ;; )
		{			
			hdc1080_readTemperature(&hdc1080_Temp, &dev_Sensor);
			hdc1080_readHumidity(&hdc1080_Humidity, &dev_Sensor);
					
			adcVal = ADC0_read();
			temp_C = temperatureConvert(adcVal);

			//printf("--> TASK vSensorTask: Temp (ADC): %d HDC1080 T: %.02f H: %.02f\n\r", temp_C, hdc1080_Temp, hdc1080_Humidity);
			
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}
	
	/**************************************************************************//**
	* \fn static void vUartTask(void* pvParameters)
	*
	* \brief
	*
	* \param[in]   pvParameters
	******************************************************************************/
	static void vPwmTask(void* pvParameters)
	{
		//printf("--> TASK vPwmTask: init\n\r");
		uint16_t i = 0;
		uint16_t dutyCycle = 0;
		
		for ( ;; )
		{
/*
			printf("--> TASK vPwmTask: %d\n\r", i++);
			
			for(dutyCycle = 0; dutyCycle < PERIOD_EXAMPLE_VALUE; dutyCycle++)
			{
				setPwmDutyCycle(dutyCycle);
				vTaskDelay(pdMS_TO_TICKS(1));
			}
			
			for(dutyCycle = PERIOD_EXAMPLE_VALUE; dutyCycle > 0; dutyCycle--)
			{
				setPwmDutyCycle(dutyCycle);
				vTaskDelay(pdMS_TO_TICKS(1));
			}

			setPwmDutyCycle(0);
*/
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
#endif