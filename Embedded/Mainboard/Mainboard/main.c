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
#include "driver/INA228/ina228.h"


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

struct bme280_dev	sensorEnv;
struct bmi160_dev	sensorGyro;
struct bmm150_dev	sensorCompass;

struct ina228_dev	monitorMotor_0;
struct ina228_dev	monitorMotor_1;

//ads1115_dev	analogSensor_0
//ads1115_dev	analogSensor_1

//pca9685_dev	pwmControl

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


int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	
	rslt = I2C_0_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_0_getData(dev_addr, (uint8_t *)reg_data, (uint8_t)len);
	
	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	
	rslt = I2C_0_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_0_sendData(dev_addr, reg_data, (uint8_t)len);
	
	return rslt;
}

int8_t user_i2c_1_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	
	rslt = I2C_1_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_1_getData(dev_addr, (uint8_t *)reg_data, (uint8_t)len);
	
	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_1_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	
	rslt = I2C_1_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_1_sendData(dev_addr, reg_data, (uint8_t)len);
	
	return rslt;
}

int8_t user_i2c_read_bmi160(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	int8_t rslt = 0;
	
	rslt = I2C_0_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_0_getData(dev_addr, data, (uint8_t)len);
	
	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write_bmi160(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	int8_t rslt = 0;
	
	rslt = I2C_0_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_0_sendData(dev_addr, data, (uint8_t)len);
	
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

int8_t configure_bme280(struct bme280_dev *dev)
{
	int8_t rslt;
	uint8_t settings_sel;
	struct bme280_data comp_data;

	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;

	settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

	rslt = bme280_set_sensor_settings(settings_sel, dev);
	if (rslt != BME280_OK)
	{
		fprintf(stderr, "Failed to set sensor settings (code %+d).", rslt);
	}
	return rslt;
}

void print_bme280_sensor_data(struct bme280_data *comp_data)
{
	float temp, press, hum;

	#ifdef BME280_FLOAT_ENABLE
	temp = comp_data->temperature;
	press = 0.01 * comp_data->pressure;
	hum = comp_data->humidity;
	#else
	#ifdef BME280_64BIT_ENABLE
	temp = 0.01f * comp_data->temperature;
	press = 0.0001f * comp_data->pressure;
	hum = 1.0f / 1024.0f * comp_data->humidity;
	#else
	temp = 0.01f * comp_data->temperature;
	press = 0.01f * comp_data->pressure;
	hum = 1.0f / 1024.0f * comp_data->humidity;
	#endif
	#endif
	printf("%0.2lf deg C, %0.2lf hPa, %0.2lf%%\n\r", temp, press, hum);
}

int8_t configure_bmi160(struct bmi160_dev *dev)
{
	int8_t rslt;
	bmi160_soft_reset(dev);
	/* Select the Output data rate, range of accelerometer sensor */
	dev->accel_cfg.odr = BMI160_ACCEL_ODR_400HZ;
	dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
	dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

	/* Select the power mode of accelerometer sensor */
	dev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

	/* Select the Output data rate, range of Gyroscope sensor */
	dev->gyro_cfg.odr = BMI160_GYRO_ODR_400HZ;
	dev->gyro_cfg.range = BMI160_GYRO_RANGE_250_DPS;
	dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

	/* Select the power mode of Gyroscope sensor */
	dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

	/* Set the sensor configuration */
	rslt = bmi160_set_sens_conf(dev);
	
	if (rslt != BMI160_OK)
	{
		fprintf(stderr, "Failed to set sensor settings (code %+d).", rslt);
	}
	return rslt;
}

static int8_t configure_bmm150(struct bmm150_dev *dev)
{
	/* Status of api are returned to this variable. */
	int8_t rslt;

	struct bmm150_settings settings;

	/* Read the default configuration from the sensor */
	settings.pwr_mode = BMM150_POWERMODE_FORCED; //BMM150_POWERMODE_NORMAL;
	settings.preset_mode = 0x00;
	settings.xyz_axes_control = 0x3F;
	
	rslt = bmm150_set_op_mode(&settings, dev);

	if (rslt == BMM150_OK)
	{
		/* Set any threshold level below which low threshold interrupt occurs */
		settings.int_settings.low_threshold = 0x0A;
		rslt = bmm150_set_sensor_settings(BMM150_SEL_LOW_THRESHOLD_SETTING, &settings, dev);
	}

	return rslt;
}

static int8_t get_compass_data(struct bmm150_dev *dev)
{
	/* Status of api are returned to this variable. */
	int8_t rslt;

	struct bmm150_mag_data mag_data;

	/* Get the interrupt status */
	rslt = bmm150_get_interrupt_status(dev);

	/* Read mag data */
	rslt = bmm150_read_mag_data(&mag_data, dev);

	/* Unit for magnetometer data is microtesla(uT) */
	printf("MAG DATA  X : %d uT   Y : %d uT   Z : %d uT\n\r", mag_data.x, mag_data.y, mag_data.z);

	return rslt;
}


int main(void)
{
	// setup our stdio stream
	stdout = &mystdout;

	CLOCK_OSCHF_crystal_PLL_init(CLKCTRL_FREQSEL_24M_gc);

	PORT_init();
	//TCA1_init();
	
	USART0_init(PIN4_bm, PIN5_bm, 1, 115200);
	
//	VREF0_init();
//	ADC0_init();
//	ADC0_start();
	
	I2C_0_init(PORTMUX_TWI0_DEFAULT_gc, I2C_SCL_FREQ);
	I2C_1_init(PORTMUX_TWI1_ALT2_gc, I2C_SCL_FREQ);	
				
	I2C_0_scan(0x01, 0x7F);
	I2C_1_scan(0x01, 0x7F);
			
	printf("I2C_0:												\n\r	\
			BMM150_I2C_ADDRESS_CSB_HIGH_SDO_HIGH		0x13	\n\r	\
			ADS1115_IIC_ADDRESS_0						0x48	\n\r	\
			ADS1115_IIC_ADDRESS_1						0x49	\n\r	\
			BMI160_I2C_ADDR_SEC							0x69	\n\r	\
			BME280_I2C_ADDR_SEC							0x77	\n\r	\
			I2C_1:												\n\r	\
			INA228										0x40	\n\r	\
			INA228										0x41	\n\r	\
			PCA9685_A0_A1								0x43	\n\r");
	
	sei();

	/************************************************************************/
	/* I2C Sensor - Driver Config											*/
	/************************************************************************/
	
	uint8_t dev_addr_sensorEnv = BME280_I2C_ADDR_SEC;
	sensorEnv.intf_ptr = &dev_addr_sensorEnv;
	sensorEnv.intf = BME280_I2C_INTF;
	sensorEnv.read = user_i2c_read;
	sensorEnv.write = user_i2c_write;
	sensorEnv.delay_us = user_delay_us;

	sensorGyro.id = (BMI160_I2C_ADDR + 1);
	sensorGyro.intf = BMI160_I2C_INTF;
	sensorGyro.read = user_i2c_read_bmi160;
	sensorGyro.write = user_i2c_write_bmi160;
	sensorGyro.delay_ms = user_delay_ms;
	
	uint8_t dev_addr_sensorCompass = BMM150_I2C_ADDRESS_CSB_HIGH_SDO_HIGH;
	sensorCompass.intf_ptr = &dev_addr_sensorCompass;
	sensorCompass.intf = BMM150_I2C_INTF;
	sensorCompass.read = user_i2c_read;
	sensorCompass.write = user_i2c_write;
//	sensorCompass.delay_us = user_delay_us;

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

	printf("\n***************************************************\n\r");
	
	int8_t result = 0;
	result = bme280_init(&sensorEnv);
	printf("BOSCH BME280 INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, sensorEnv.chip_id);
	
	result = bmi160_init(&sensorGyro);
	printf("BOSCH BMI160 INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, sensorGyro.chip_id);
	
	result = bmm150_init(&sensorCompass);
	printf("BOSCH BMM150 INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, sensorCompass.chip_id);
	
	result = ina228_init(&monitorMotor_0);
	printf("TI INA228 [0] INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, monitorMotor_0.devID);
	result = ina228_init(&monitorMotor_1);
	printf("TI INA228 [1] INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, monitorMotor_1.devID);

	printf("***************************************************\n\r");

	/************************************************************************/
	/* Sensorsettings                                                       */
	/************************************************************************/

	result = configure_bme280(&sensorEnv);
	result = configure_bmi160(&sensorGyro);
	result = configure_bmm150(&sensorCompass);
	
	PORTC_set_pin_level(MOTOR_EN_M1, true);
	PORTC_set_pin_level(MOTOR_EN_M5, true);
	
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
	
		int8_t rslt;
		int16_t temp_C;
		uint16_t adcVal;
		float hdc1080_Temp = 0;
		float hdc1080_Humidity = 0;
		uint16_t dutyCycle = 0;
	
		struct bme280_data sensEnvData;
		
		/*! @brief variable to hold the bmi160 accel data */
		struct bmi160_sensor_data bmi160_accel;

		/*! @brief variable to hold the bmi160 gyro data */
		struct bmi160_sensor_data bmi160_gyro;

		while (1)
		{
			rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &sensorEnv);
			
//			adcVal = ADC0_read();
			temp_C = temperatureConvert(adcVal);
			float voltage_0, voltage_1;
			float dieTemp_0, dieTemp_1;
			ina228_voltage(&voltage_0, &monitorMotor_0);
			ina228_voltage(&voltage_1, &monitorMotor_1);
			ina228_dietemp(&dieTemp_0, &monitorMotor_0);
			ina228_dietemp(&dieTemp_1, &monitorMotor_1);
			
			printf("M0: V: %05dmV T: %04d M1: V: %05dmV  T: %04d\t", (uint16_t)(voltage_0*1000), (uint16_t)(dieTemp_0 * 10), (uint16_t)(voltage_1 * 1000), (uint16_t)(dieTemp_1 * 10) );
			
			bme280_get_sensor_data(BME280_ALL, &sensEnvData, &sensorEnv);
			print_bme280_sensor_data(&sensEnvData);
			
			/* To read both Accel and Gyro data */
			bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL | BMI160_TIME_SEL), &bmi160_accel, &bmi160_gyro, &sensorGyro);

			printf("ax:%d\tay:%d\taz:%d\t", bmi160_accel.x, bmi160_accel.y, bmi160_accel.z);
			printf("gx:%d\tgy:%d\tgz:%d\n\r", bmi160_gyro.x, bmi160_gyro.y, bmi160_gyro.z);
			
			get_compass_data(&sensorCompass);
			
			_delay_ms(500);
		
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