/*
The MIT License (MIT)

Copyright (c) 2015 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _OPT3001_
#define _OPT3001_

#include <stdint.h>

#define OPT3001_ADDRESS_GND 0x44
#define OPT3001_ADDRESS_VCC 0x45
#define OPT3001_ADDRESS_SDA 0x46
#define OPT3001_ADDRESS_SCL 0x47
typedef enum
{
	RESULT		= 0x00,
	CONFIG		= 0x01,
	LOW_LIMIT	= 0x02,
	HIGH_LIMIT	= 0x03,

	MANUFACTURER_ID = 0x7E,
	DEVICE_ID		= 0x7F,
} OPT3001_Commands;

typedef enum
{
	OPT3001_NO_ERROR = 0,
	OPT3001_NULL_PTR = -1,
	OPT3001_DEV_NOT_FOUND = -2,
	
	
	OPT3001_TIMEOUT_ERROR = -100,

	// Wire I2C translated error codes
	OPT3001_COMM_FAIL = -10,
	OPT3001_INVALID_LEN = -11,
	OPT3001_WIRE_I2C_DATA_TOO_LOG = -12,
	OPT3001_WIRE_I2C_RECEIVED_NACK_ON_ADDRESS = -13,
	OPT3001_WIRE_I2C_RECEIVED_NACK_ON_DATA = -14,
	OPT3001_WIRE_I2C_UNKNOW_ERROR = -15
} OPT3001_ErrorCode;

typedef union
{
	uint16_t rawData;
	struct
	{
		uint16_t Result : 12;
		uint8_t Exponent : 4;
	};
} OPT3001_ER;

typedef union
{
	struct
	{
		uint8_t FaultCount : 2;
		uint8_t MaskExponent : 1;
		uint8_t Polarity : 1;
		uint8_t Latch : 1;
		uint8_t FlagLow : 1;
		uint8_t FlagHigh : 1;
		uint8_t ConversionReady : 1;
		uint8_t OverflowFlag : 1;
		uint8_t ModeOfConversionOperation : 2;
		uint8_t ConvertionTime : 1;
		uint8_t RangeNumber : 4;		
	};
	uint16_t rawData;
} OPT3001_Config;

struct OPT3001_Value
{
	float lux;
	OPT3001_ER raw;
	OPT3001_ErrorCode error;
};

/* type definitions */
typedef int8_t (*opt3001_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint16_t devAddr);
typedef int8_t (*opt3001_write_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint16_t devAddr);
typedef void (*opt3001_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

struct opt3001_dev
{
	/*! Device Address */
	uint8_t dev_address;

	/*< Read function pointer */
	opt3001_read_fptr_t read;

	/*< Write function pointer */
	opt3001_write_fptr_t write;

	/*< Delay function pointer */
	opt3001_delay_us_fptr_t delay_us;

	/*< Sensor settings */
	uint16_t settings;

	/*< Variable to store result of read/write function */
	int8_t intf_rslt;
};

int16_t opt3001_init(struct opt3001_dev *dev);

int16_t opt3001_readManufacturerID(uint16_t *value, struct opt3001_dev *dev);
int16_t opt3001_readDeviceID(uint16_t *value, struct opt3001_dev *dev);

int16_t opt3001_readResult(struct OPT3001_Value *value, struct opt3001_dev *dev);
int16_t opt3001_readHighLimit(struct OPT3001_Value *value, struct opt3001_dev *dev);
int16_t opt3001_readLowLimit(struct OPT3001_Value *value, struct opt3001_dev *dev);
	
int16_t opt3001_readConfig(OPT3001_Config *config, struct opt3001_dev *dev);
int16_t opt3001_writeConfig(OPT3001_Config config, struct opt3001_dev *dev);

int8_t opt3001_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, struct opt3001_dev *dev);
int8_t opt3001_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct opt3001_dev *dev);
int16_t opt3001_readRegister(OPT3001_Commands command, struct OPT3001_Value *value, struct opt3001_dev *dev);

int8_t opt3001_null_ptr_check(const struct opt3001_dev *dev);

#endif 