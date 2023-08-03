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
#include "commonDataMgmt.h"

#include "driver/BME280/bme280.h"
#include "driver/BME280/bme280_defs.h"

#include "driver/BMI160/bmi160.h"
#include "driver/BMI160/bmi160_defs.h"

#include "driver/BMM150/bmm150.h"
#include "driver/BMM150/bmm150_defs.h"

#include "driver/TMAG5273/tmag5273.h"
#include "driver/ADS1115/ads1115.h"

#include "taskEnvSensor.h"

// ********************************************************************************
// Macros and Defines
// ********************************************************************************


// ********************************************************************************
// Variables
// ********************************************************************************

struct bme280_dev	sensorEnv;
struct bmi160_dev	sensorGyro;
struct bmm150_dev	sensorCompass;

struct bmm150_mag_data magData;

ads1115_t	analogSensor_0;
ads1115_t	analogSensor_1;

struct tmag5273_dev magnetSensor;

struct bme280_float_data
{
	/*< Compensated pressure */
	float pressure;

	/*< Compensated temperature */
	float temperature;

	/*< Compensated humidity */
	float humidity;
};

int8_t readAnalog(enum ADC_MUXPOS_enum channel, uint16_t *value)
{
	ADC0_selectChannel(channel);
	
	*value = ADC0_read();
	return 0;
}

int8_t configure_bme280(struct bme280_dev *dev)
{
	int8_t rslt;
	uint8_t settings_sel;

	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;

	settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

	rslt = bme280_set_sensor_settings(settings_sel, dev);

	return rslt;
}

void convert_bme280_sensor_data2float(struct bme280_data *comp_data, struct bme280_float_data *convData)
{
	convData->temperature = 0.01f * comp_data->temperature;
	convData->pressure = 0.01f * comp_data->pressure;
	convData->humidity = 1.0f / 1024.0f * comp_data->humidity;
}

int8_t configure_bmi160(struct bmi160_dev *dev)
{
	int8_t rslt;

	/* Select the power mode of accelerometer sensor */
	dev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
	/* Select the Output data rate, range of accelerometer sensor */
	dev->accel_cfg.odr = BMI160_ACCEL_ODR_100HZ;
	dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
	dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

	/* Select the power mode of Gyroscope sensor */
	dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;
	/* Select the Output data rate, range of Gyroscope sensor */
	dev->gyro_cfg.odr = BMI160_GYRO_ODR_100HZ;
	dev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
	dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

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

static int8_t get_compass_data(struct bmm150_dev *dev, struct bmm150_mag_data *mag_data)
{
	/* Status of api are returned to this variable. */
	int8_t rslt;

	/* Get the interrupt status */
	rslt = bmm150_get_interrupt_status(dev);

	/* Read mag data */
	rslt = bmm150_read_mag_data(mag_data, dev);

	return rslt;
}

uint8_t buffer[300];
/**************************************************************************//**
* \fn static void vUartTask(void* pvParameters)
*
* \brief
*
* \param[in]   pvParameters
******************************************************************************/
void vEnvSensorTask(void* pvParameters)
{
	static const char msg[] = "--> vSensTask: started...\n\r";
	xMessageBufferSend(terminal_tx_buffer, msg, sizeof(msg), 1000);

	uint16_t adcVal;
	
	
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
	sensorCompass.delay_us = user_delay_us;
		
	analogSensor_0.DevAddr = 0x48;
	analogSensor_0.Read = user_i2c_read_ads1115;
	analogSensor_0.Write = user_i2c_write_ads1115;
	
	analogSensor_1.DevAddr = 0x49;
	analogSensor_1.Read = user_i2c_read_ads1115;
	analogSensor_1.Write = user_i2c_write_ads1115;
	
	magnetSensor.i2c_addr = TMAG5273A1_I2C_DEF_ADDR;
	magnetSensor.read = user_i2c_read_bmi160;
	magnetSensor.write = user_i2c_write_bmi160;
	
	tmag5273_init(&magnetSensor);
	sprintf((char*)buffer, "--> vSensTask: TMAG5273A1 INIT:	%d --> DEVICE_ID: 0x%02X  MAN_ID: 0x%04X\n\r", magnetSensor.rslt, magnetSensor.deviceID, magnetSensor.manufacturerID);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
	
	bool status_0 = ads1115_Open(&analogSensor_0, ADS1115_ADDRESS1, user_i2c_read_ads1115, user_i2c_write_ads1115);
	bool status_1 = ads1115_Open(&analogSensor_1, ADS1115_ADDRESS2, user_i2c_read_ads1115, user_i2c_write_ads1115);

	sprintf((char*)buffer, "--> vSensTask: ADS1115 [0] OPEN:	%d ADS1115 [1] OPEN:	%d\n\r", (int8_t)status_0, (int8_t)status_1);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
	
	ADC0_init(VREF_REFSEL_2V500_gc);
	ADC0_start();
	
	int8_t result = bme280_init(&sensorEnv);
	sprintf((char*)buffer, "--> vSensTask: BOSCH BME280 INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, sensorEnv.chip_id);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);

	result = bmi160_init(&sensorGyro);
	sprintf((char*)buffer, "--> vSensTask: BOSCH BMI160 INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, sensorGyro.chip_id);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
		
	result = bmm150_init(&sensorCompass);
	sprintf((char*)buffer, "--> vSensTask: BOSCH BMM150 INIT:	%d --> CHIP_ID: 0x%02X\n\r", result, sensorCompass.chip_id);
	xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
	
	struct bme280_data sensEnvData;
	struct bme280_float_data convData;
	
	/*! @brief variable to hold the bmi160 accel data */
	struct bmi160_sensor_data bmi160_accel;

	/*! @brief variable to hold the bmi160 gyro data */
	struct bmi160_sensor_data bmi160_gyro;
	
	configure_bme280(&sensorEnv);
	configure_bmi160(&sensorGyro);
	configure_bmm150(&sensorCompass);
	
	for ( ;; )
	{
		bme280_set_sensor_mode(BME280_FORCED_MODE, &sensorEnv);
		configure_bmm150(&sensorCompass);
					
		readAnalog(ADC_MUXPOS_AIN19_gc, &adcVal);
//		uint32_t adcVoltage = ( (uint32_t)adcVal * 2500 ) / 4096;

		int16_t adc_0 = ads1115_ConvertOnce(&analogSensor_0, ADS1115_MUX_AIN0_GND, ADS1115_PGA_2p048V);
		int16_t adc_1 = ads1115_ConvertOnce(&analogSensor_1, ADS1115_MUX_AIN0_GND, ADS1115_PGA_2p048V);

		//sprintf((char*)buffer, "--> vSensTask: ADS1115 -> [0] %04d  [1] %04d\n\r", adc_0, adc_1);
		//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 500);

		bme280_get_sensor_data(BME280_ALL, &sensEnvData, &sensorEnv);
		convert_bme280_sensor_data2float(&sensEnvData, &convData);
			
		/* To read both Accel and Gyro data */
		bmi160_get_sensor_data((BMI160_ACCEL_SEL | BMI160_GYRO_SEL | BMI160_TIME_SEL), &bmi160_accel, &bmi160_gyro, &sensorGyro);

		//sprintf((char*)buffer, "--> vSensTask: ACC--> ax:%04d\tay:%04d\taz:%04d\n\r", bmi160_accel.x, bmi160_accel.y, bmi160_accel.z);
		//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
		//sprintf((char*)buffer, "--> vSensTask: GYR--> gx:%04d\tgy:%04d\tgz:%04d\n\r", bmi160_gyro.x, bmi160_gyro.y, bmi160_gyro.z);
		//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
			
		get_compass_data(&sensorCompass, &magData);
		//sprintf((char*)buffer, "--> vSensTask: MAG X : %04duT Y : %04duT Z : %04duT\n\r", magData.x, magData.y, magData.z);
		//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);	
			
		float magTemp = 0.0f;
		float magAng = 0.0f;
		struct tmag5273_raw_sensor_data magSensorVector;
			
		readTemperatureData(&magTemp, &magnetSensor);
		readAngleData(&magAng, &magnetSensor);
		readRawXYZData(&magSensorVector, &magnetSensor);

		//sprintf((char*)buffer, "--> vSensTask: TMAG--> T: %.02f, A: %.03f, Vec[x,y,z] %04d %04d %04d\n\r", magTemp, magAng, magSensorVector.x, magSensorVector.y, magSensorVector.z);
		//xMessageBufferSend(terminal_tx_buffer, buffer, sizeof(buffer), 100);
		
		glbRoboterData.dataAcc[0] = bmi160_accel.x;
		glbRoboterData.dataAcc[1] = bmi160_accel.y;
		glbRoboterData.dataAcc[2] = bmi160_accel.z;
		
		glbRoboterData.dataGyro[0] = bmi160_gyro.x;
		glbRoboterData.dataGyro[1] = bmi160_gyro.y;
		glbRoboterData.dataGyro[2] = bmi160_gyro.z;
		
		glbRoboterData.dataCompass[0] = magData.x;
		glbRoboterData.dataCompass[1] = magData.y;
		glbRoboterData.dataCompass[2] = magData.z;
		
		glbRoboterData.temperature = convData.temperature;
		glbRoboterData.humidity = convData.humidity;
		glbRoboterData.pressure = convData.pressure;
		
		glbRoboterData.analogSensor[0] = adc_0;
		glbRoboterData.analogSensor[4] = adc_1;
		
		glbRoboterData.magnetSensor[0] = magSensorVector.x;
		glbRoboterData.magnetSensor[0] = magSensorVector.y;
		glbRoboterData.magnetSensor[0] = magSensorVector.z;
		
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}