/*

Arduino Library for Texas Instruments HDC1080 Digital Humidity and Temperature Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2016 ClosedCube Limited

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

#ifndef _HDC1080_h
#define _HDC1080_h

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* type definitions */
typedef int8_t (*hdc1080_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint16_t devAddr);
typedef int8_t (*hdc1080_write_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint16_t devAddr);
typedef void (*hdc1080_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

#define HDC1080_IIC_ADDRESS			0x40

#define	HDC1080_TEMPERATURE			0x00
#define	HDC1080_HUMIDITY			0x01
#define	HDC1080_CONFIGURATION		0x02
#define	HDC1080_MANUFACTURER_ID		0xFE
#define	HDC1080_DEVICE_ID			0xFF
#define	HDC1080_SERIAL_ID_FIRST		0xFB
#define	HDC1080_SERIAL_ID_MID		0xFC
#define	HDC1080_SERIAL_ID_LAST		0xFD

/**\name API success code */
#define HDC1080_OK					(int8_t)(0)

/**\name API error codes */
#define HDC1080_E_NULL_PTR			(int8_t)(-1)
#define HDC1080_E_DEV_NOT_FOUND		(int8_t)(-2)
#define HDC1080_E_INVALID_LEN		(int8_t)(-3)
#define HDC1080_E_COMM_FAIL			(int8_t)(-4)
#define HDC1080_E_SLEEP_MODE_FAIL	(int8_t)(-5)
#define HDC1080_E_NVM_COPY_FAILED	(int8_t)(-6)

#define HDC1080_CONFIG_RST			(1UL << (14))
#define HDC1080_CONFIG_HEAT_EN		(1UL << (13))
#define HDC1080_CONFIG_MODE_EN		(1UL << (12))		
#define HDC1080_CONFIG_BTST_EN		(1UL << (11))		
#define HDC1080_CONFIG_TRES_14BIT	0x00
#define HDC1080_CONFIG_TRES_11BIT	(1UL << (10))
#define HDC1080_CONFIG_HRES_14BIT	0x00
#define HDC1080_CONFIG_HRES_11BIT	(1UL << (9))
#define HDC1080_CONFIG_HRES_8BIT	(1UL << (8))

struct hdc1080_dev
{
    /*! Device Address */
    uint8_t dev_address;

    /*< Read function pointer */
    hdc1080_read_fptr_t read;

    /*< Write function pointer */
    hdc1080_write_fptr_t write;

    /*< Delay function pointer */
    hdc1080_delay_us_fptr_t delay_us;

    /*< Sensor settings */
    uint16_t settings;

    /*< Variable to store result of read/write function */
    int8_t intf_rslt;
};

int8_t null_ptr_check(const struct hdc1080_dev *dev);

int8_t hdc1080_init(struct hdc1080_dev *dev);

int8_t hdc1080_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, struct hdc1080_dev *dev);
int8_t hdc1080_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct hdc1080_dev *dev);

int8_t hdc1080_get_sensor_settings(uint16_t *settings, struct hdc1080_dev *dev);
int8_t hdc1080_set_sensor_settings(uint16_t settings, struct hdc1080_dev *dev);

int8_t hdc1080_readManufacturerId(uint16_t *value, struct hdc1080_dev *dev);	// 0x5449 ID of Texas Instruments
int8_t hdc1080_readDeviceId(uint16_t *value, struct hdc1080_dev *dev);			// 0x1050 ID of the device

int8_t hdc1080_triggerMeasurement( struct hdc1080_dev *dev);

int8_t hdc1080_readTemperature(float *value, struct hdc1080_dev *dev);
int8_t hdc1080_readHumidity(float *value, struct hdc1080_dev *dev);

#endif