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
		
		dev->rslt += TMAG5x73getXYZrange(&dev->rangeXY, &dev->rangeZ, dev);
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

int8_t readRawXYZData(struct tmag5273_raw_sensor_data *vector, struct tmag5273_dev *dev)
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
	uint8_t data[6];
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_X_MSB_RESULT, (uint8_t*)&data, 6);
	
	vector->x = (float)( ( (int16_t)data[0] << 8) | (int16_t)data[1] );
	vector->y = (float)( ( (int16_t)data[2] << 8) | (int16_t)data[3] );
	vector->z = (float)( ( (int16_t)data[4] << 8) | (int16_t)data[5] );
	
	vector->x = (vector->x / 32768) * dev->rangeXY;
	vector->y = (vector->x / 32768) * dev->rangeXY;
	vector->z = (vector->x / 32768) * dev->rangeZ;
	
	return dev->rslt;
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

//****************************************************************************
//! Get and return the integer value of the X_Y_RANGE bits for an axis
//!
//! Returns an unsigned 16-bit integer value of the X axis range in mT.
//****************************************************************************
int8_t TMAG5x73getXYZrange(uint16_t *rangeXY, uint16_t *rangeZ, struct tmag5273_dev *dev)
{
    // Get SENSOR_CONFIG_2 and isolate X_Y_RANGE bits.
	uint8_t data = 0;
	uint8_t configXY = 0;
	uint8_t configZ = 0;
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_SENSOR_CONFIG_2, (uint8_t*)&data, 1);
	configXY = (data  & SENSOR_CONFIG_2_X_Y_RANGE_MASK) >> 1;
	configZ = (data  & SENSOR_CONFIG_2_X_Y_RANGE_MASK);
	
	uint8_t version = 0;
	TMAG5x73getVersion(&version, dev);
	
    if (version == 2)
    {
        // range values for TMAG5x73A2
        *rangeXY = 133;
		*rangeZ = 133;
        if (configXY == 0x1) *rangeXY = 266; // If examined bits equal 1b, range is set to 266 mT (for A2)
        else *rangeXY = 133; // If examined bits equal 0b, range is set to 133 mT (for A2)
        
		if (configZ == 0x1) *rangeZ = 266; // If examined bits equal 1b, range is set to 266 mT (for A2)
        else *rangeZ = 133; // If examined bits equal 0b, range is set to 133 mT (for A2)
    }
    else
    {
        // range values for TMAG5x73A1
        *rangeXY = 40;
		*rangeZ = 40;
        if (configXY == 0x1) *rangeXY = 80; // If examined bits equal 1b, range is set to 80 mT (for A1)
        else *rangeXY = 40; // If examined bits equal 0b, range is set to 40 mT (for A1)
		
		if (configZ == 0x1) *rangeZ = 80; // If examined bits equal 1b, range is set to 80 mT (for A1)
		else *rangeZ = 40; // If examined bits equal 0b, range is set to 40 mT (for A1)
    }
	
    return dev->rslt;
}

//****************************************************************************
//! Get TMAG5x73 Version (A1 or A2)
//!
//! Sends a read command for TEST_CONFIG and returns the VER field bit.
//!      VER == 0x00 --> TMAG5x73APL
//!      VER == 0x01 --> TMAG5x73A1
//!      VER == 0x02 --> TMAG5x73A2
//****************************************************************************
int8_t TMAG5x73getVersion(uint8_t *version, struct tmag5273_dev *dev)
{
	uint8_t data;
	dev->rslt = dev->read(dev->i2c_addr, TMAG5273_DEVICE_ID, (uint8_t*)&data, 1);
	*version = data & DEVICE_ID_VER_MASK;
	
	return dev->rslt;
}

