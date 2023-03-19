/*
 * tmag5273.c
 *
 * Created: 18.03.2023 22:05:11
 *  Author: jakob
 */ 

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "tmag5273.h"

int8_t tmag5273_init(struct tmag5273_dev *dev)
{
	uint8_t data = 0;
	dev->rslt = tmag5273_getDeviceID(&dev->deviceID, dev);
	dev->rslt += tmag5273_getManufacturerID(&dev->manufacturerID, dev);
	
	if (dev->manufacturerID == TMAG5273_MANUFACTURER_ID)
	{
		// CRC disable; NO T compensation; 1x Avrg (10kSPS); I2C 3bytes read Command
		data = 0x00;
		dev->rslt = dev->write(dev->i2c_addr, TMAG5273_DEVICE_CONFIG_1, &data, 1);
	
		// THR_HYST = 0; Low active current mode; I2C glitch filter; Conversion Start at I2C; Continuous measure mode
		data = 0x02;
		dev->rslt += dev->write(dev->i2c_addr, TMAG5273_DEVICE_CONFIG_2, &data, 1);
	
		// X, Y, Z channel enabled; Sleeptime 4h = 20ms
		data = 0x74;
		dev->rslt += dev->write(dev->i2c_addr, TMAG5273_SENSOR_CONFIG_1, &data, 1);
	
		// Enables angle calculation --> X 1st, Y 2nd
		data = 0x04;
		dev->rslt += dev->write(dev->i2c_addr, TMAG5273_SENSOR_CONFIG_2, &data, 1);

		//Temp channel enabled
		data = 0x01;
		dev->rslt += dev->write(dev->i2c_addr, TMAG5273_T_CONFIG, &data, 1);
	}
	else
	{
		dev->rslt = -5;
	}
	
	return dev->rslt; 
}

int8_t tmag5273_setDeviceConfig(uint16_t config, struct tmag5273_dev *dev)
{
	uint8_t data[2];
	
	data[0] = (uint8_t)( config & 0x00FF);
	data[1] = (uint8_t)( (config >> 8) & 0x00FF);
	
	dev->rslt = dev->write(dev->i2c_addr, TMAG5273_DEVICE_CONFIG_1, (uint8_t*)&data, 2);
	
	return dev->rslt; 
}

int8_t tmag5273_setSensorConfig(uint16_t config, struct tmag5273_dev *dev)
{
	uint8_t data[2];
	
	data[0] = (uint8_t)( config & 0x00FF);
	data[1] = (uint8_t)( (config >> 8) & 0x00FF);
	
	dev->rslt = dev->write(dev->i2c_addr, TMAG5273_SENSOR_CONFIG_1, (uint8_t*)&data, 2);
	
	return dev->rslt;
}

int8_t tmag5273_setXYZ_ThrConfig(struct tmag5273_XYZ_threshold thr, struct tmag5273_dev *dev)
{
	
	dev->rslt = dev->write(dev->i2c_addr, TMAG5273_X_THR_CONFIG, (uint8_t*)&thr, 3);
	
	return dev->rslt;
	return -1;
}

int8_t tmag5273_setTempConfig(uint8_t config, struct tmag5273_dev *dev)
{
	dev->rslt = dev->write(dev->i2c_addr, TMAG5273_T_CONFIG, (uint8_t*)&config, 1);
	
	return dev->rslt;
	return -1;
}

int8_t tmag5273_getDeviceID(uint8_t *devID, struct tmag5273_dev *dev)
{
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_DEVICE_ID, (uint8_t*)devID, 1);
	return dev->rslt;
}

int8_t tmag5273_getManufacturerID(uint16_t *manID, struct tmag5273_dev *dev)
{
	uint8_t data[2];
	
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_MANUFACTURER_ID_LSB, (uint8_t*)&data, 2);
	*manID = ( ((uint16_t)data[1] << 8 ) | (uint16_t)data[0] );
	
	return dev->rslt;
}

int8_t readTemperatureData(float *temp, struct tmag5273_dev *dev)
{
	uint8_t data[2];
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_T_MSB_RESULT, (uint8_t*)&data, 2);
	  
	int16_t temperatureData = ( ( (int16_t)data[0] << 8) | (int16_t)data[1] ); //16bits that make up the temperature data
	
	float TADCT = (float)temperatureData;
	float TSENSET0 = 25;
	float TADCRES = 60.1;
	float TADCT0 = 17508;
	float T = TSENSET0 + ((TADCT - TADCT0) / (TADCRES));
	
	*temp = T;
	
	return dev->rslt;
}

int8_t readRawXYZData(struct tmag5273_sensor_data *vector, struct tmag5273_dev *dev)
{
	uint8_t data[6];
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_X_MSB_RESULT, (uint8_t*)&data, 6);
	
	vector->x = ( ( (int16_t)data[0] << 8) | (int16_t)data[1] );
	vector->y = ( ( (int16_t)data[2] << 8) | (int16_t)data[3] );
	vector->z = ( ( (int16_t)data[4] << 8) | (int16_t)data[5] );
	
	return dev->rslt;
}

int8_t readXYZData(struct tmag5273_sensor_data *vector, struct tmag5273_dev *dev)
{
	
}

int8_t readRawAngleData(uint16_t *angle, struct tmag5273_dev *dev)
{
	uint8_t data[2];
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_ANGLE_RESULT_MSB, (uint8_t*)&data, 2);
	
	uint16_t result = ( ( (uint16_t)data[0] << 8) | (uint16_t)data[1] );
	
	*angle = result;
	
	return dev->rslt;
}

int8_t readAngleData(float *angle, struct tmag5273_dev *dev)
{
	uint8_t data[2];
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_ANGLE_RESULT_MSB, (uint8_t*)&data, 2);
	
	uint16_t result = ( ( (uint16_t)data[0] << 8) | (uint16_t)data[1] );
	
	float ang = (float)(result >> 4);				// 360deg;
	ang += (float)(result & 0x000F) * 0.0625;		// last 4bit -> 1/16;
	
	*angle = ang;
	
	return dev->rslt;
}

int8_t readRawMagnitudeData(uint8_t *magnitude, struct tmag5273_dev *dev)
{
	uint8_t data;
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_MAGNITUDE_RESULT, (uint8_t*)&data, 1);

	*magnitude = data;
	
	return dev->rslt;
}
