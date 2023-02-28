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

#include "driver/hdc1080/hdc1080.h"
#include "driver/ws2812/ws2812.h"
#include "driver/OLED_SSD1306/ssd1306.h"
#include "driver/OLED_SSD1306/Icons_16x16.h"

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

static struct hdc1080_dev dev_Sensor;
static struct ssd1306_dev_t display;
ws2812_t ws2812Interface;

struct cRGB colors[MAXPIX];
struct cRGB led[MAXPIX];
struct cRGB color = {
	.b = 0,
	.g = 0,
	.r = 0
};

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

void setRainbow(struct cRGB *colorArray)
{
	//Rainbowcolors
	colorArray[0].r=150; colorArray[0].g=150; colorArray[0].b=150;
	colorArray[1].r=255; colorArray[1].g=000; colorArray[1].b=000;	//red
	colorArray[2].r=255; colorArray[2].g=100; colorArray[2].b=000;	//orange
	colorArray[3].r=100; colorArray[3].g=255; colorArray[3].b=000;	//yellow
	colorArray[4].r=000; colorArray[4].g=255; colorArray[4].b=000;	//green
	colorArray[5].r=000; colorArray[5].g=100; colorArray[5].b=255;	//light blue (türkis)
	colorArray[6].r=000; colorArray[6].g=000; colorArray[6].b=255;	//blue
	colorArray[7].r=100; colorArray[7].g=000; colorArray[7].b=255;	//violet

	for (int i = 0; i < 8; i++)
	{
		led[i] = colorArray[i];
		rgbCorrectValence_ws2812(&led[i]);
	}

}

static uint8_t j = 1;
static uint8_t k = 1;
void rotateColors(void)
{
	//shift all vallues by one led
	uint8_t i=0;
	for(i=MAXPIX; i>1; i--)
	led[i-1]=led[i-2];
	
	//change colour when colourlength is reached
	if(k>COLORLENGTH)
	{
		j++;
		if(j>7)
		{
			j=0;
		}

		k=0;
	}
	k++;
	//loop colouers
	
	//fade red
	if(led[0].r<(colors[j].r-FADE))
	led[0].r+=FADE;
	
	if(led[0].r>(colors[j].r+FADE))
	led[0].r-=FADE;

	if(led[0].g<(colors[j].g-FADE))
	led[0].g+=FADE;
	
	if(led[0].g>(colors[j].g+FADE))
	led[0].g-=FADE;

	if(led[0].b<(colors[j].b-FADE))
	led[0].b+=FADE;
	
	if(led[0].b>(colors[j].b+FADE))
	led[0].b-=FADE;
	
	//ws2812_Write(&ws2812Interface, (uint8_t *)led, MAXPIX*3);
	ws2812_Write(&ws2812Interface, (uint8_t *)led, sizeof(colors) );
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
	
	ws2812_Open(&ws2812Interface, &VPORTC_OUT, PIN0_bm);
	setRainbow(colors);
	
	sei();
	
	display.id = SSD1306_I2C_ADDRESS;
	display.write = user_i2c_write_ssd1306;
	display.delay_ms = user_delay_ms;
	display.vccstate = SSD1306_SWITCHCAPVCC;
		
	SSD1306_init(&display);
	SSD1306_ClearDisplay();
	SSD1306_Update(&display);	// write displaybuffer to display
	
	dev_Sensor.dev_address = HDC1080_IIC_ADDRESS;
	dev_Sensor.settings = 0x3000;
	dev_Sensor.read = user_i2c_read_HDC1080;
	dev_Sensor.write = user_i2c_write;
	dev_Sensor.delay_us = user_delay_us;
	
	uint16_t ManufacID = 0;
	uint16_t DeviceId = 0;
	
	printf("\n***************************************************\n\r");
	hdc1080_init(&dev_Sensor);
	hdc1080_readManufacturerId(&ManufacID, &dev_Sensor);
	hdc1080_readDeviceId(&DeviceId, &dev_Sensor);
	printf("HDC Manufacturer ID:	%04X\n\r", ManufacID);
	printf("HDC Device ID:			%04X\n\r", DeviceId);
	printf("***************************************************\n\r");


	
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
			rotateColors();
		
			/*
			BUTTON_pressCallback();
			_delay_ms(50);
			BUTTON_releaseCallback();
			_delay_ms(50);
			*/
		
			hdc1080_readTemperature(&hdc1080_Temp, &dev_Sensor);
			hdc1080_readHumidity(&hdc1080_Humidity, &dev_Sensor);
		
			adcVal = ADC0_read();
			temp_C = temperatureConvert(adcVal);
		
			//USART3_sendString("Hello World!\n\r");
			printf("Temp (ADC): %d HDC1080 T: %.02f H: %.02f\n\r", temp_C, hdc1080_Temp, hdc1080_Humidity);
		
			//I2C_0_scan(0x00, 0x7F);

			uint8_t spiBuffer[20];
		
			for( int c = 0; c < 20; c++)	spiBuffer[c] = c;
			write_spi_buffer(spiBuffer, 5);
		
		
			/*
			for(dutyCycle = 0; dutyCycle < PERIOD_EXAMPLE_VALUE; dutyCycle++)
			{
				setPwmDutyCycle(dutyCycle);
				_delay_us(100);
			}
		
			for(dutyCycle = PERIOD_EXAMPLE_VALUE; dutyCycle > 0; dutyCycle--)
			{
				setPwmDutyCycle(dutyCycle);
				_delay_us(100);
			}
			*/
			//setPwmDutyCycle(0);
				
			//_delay_ms(500);		
			drawOuterFrame();
			drawAkku(3);
			drawTemeprature( (int32_t)hdc1080_Temp );
		
			SSD1306_Update(&display);
		
		}
		
	#endif
}

void drawAkku(uint8_t stat)
{
	switch(stat)
	{

		case 0: SSD1306_DrawBMP(100, 0, (uint8_t*)bat0_icon16x16, 16, 16);
		break;
		case 1: SSD1306_DrawBMP(100, 0, (uint8_t*)bat1_icon16x16, 16, 16);
		break;
		case 2: SSD1306_DrawBMP(100, 0, (uint8_t*)bat2_icon16x16, 16, 16);
		break;
		case 3: SSD1306_DrawBMP(100, 0, (uint8_t*)bat3_icon16x16, 16, 16);
		break;
		default:
		break;
	}
}

void drawOuterFrame(void)
{
	SSD1306_DrawLine(0, 0, 127, 0, true);		// ----------
	SSD1306_DrawLine(0, 63, 127, 63, true);		// |
	SSD1306_DrawLine(0, 0, 0, 63, true);		//			 |
	SSD1306_DrawLine(127, 0, 127, 63, true);	// ___________
}

void drawTemeprature(int32_t t)
{
	char str_adc[7];
	sprintf(str_adc, "%d%c", (int16_t)t, 167);
	SSD1306_DrawText(17, 1, str_adc, 2);
	
	SSD1306_DrawBMP(1, 1, temperature_icon16x16, 16, 16);
	SSD1306_DrawLine(63, 0, 63, 17, true);
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