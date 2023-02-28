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

#include "HDC1080.h"
#include <math.h>

int8_t hdc1080_init(struct hdc1080_dev *dev)
{
	uint8_t rslt = 0;

	uint16_t val = 0;
	
    rslt = hdc1080_readManufacturerId(&val, dev);
	if(val == 0x5449)
	{
		rslt = hdc1080_readDeviceId(&val, dev);
		if (val == 0x1050)
		{
			rslt = hdc1080_set_sensor_settings(dev->settings, dev);
			if (rslt != HDC1080_OK)
			{
				rslt = HDC1080_E_COMM_FAIL;
			} 
		}
		else
		{
			rslt = HDC1080_E_DEV_NOT_FOUND;
		}
	}
	else
	{
		rslt = HDC1080_E_DEV_NOT_FOUND;
	}

	return rslt;
}

int8_t hdc1080_readTemperature(float *value, struct hdc1080_dev *dev)
{
	uint8_t buf[2];
	uint16_t data = 0;

	dev->intf_rslt = hdc1080_get_regs(HDC1080_TEMPERATURE, buf, 2, dev );
	
	data = ( (uint16_t)buf[0] << 8);
	data |= ( (uint16_t)buf[1]);
	
	*value = ( (float)data / pow(2, 16) ) * 165 - 40;
	
	return dev->intf_rslt;
}

int8_t hdc1080_readHumidity(float *value, struct hdc1080_dev *dev)
{
	uint8_t buf[2];
	uint16_t data = 0;

	dev->intf_rslt = hdc1080_get_regs(HDC1080_HUMIDITY, buf, 2, dev );
		
	data = ( (uint16_t)buf[0] << 8);
	data |= ( (uint16_t)buf[1]);
	
	*value = ( (float)data / pow(2, 16)) * 100;
	
	return dev->intf_rslt;
}

int8_t hdc1080_readManufacturerId(uint16_t *value, struct hdc1080_dev *dev)
{
	uint8_t buf[2];
	
	dev->intf_rslt = hdc1080_get_regs(HDC1080_MANUFACTURER_ID, buf, 2, dev );
	
	*value = ( (uint16_t)buf[0] << 8);
	*value |= ( (uint16_t)buf[1]);
	
	return dev->intf_rslt;
}

int8_t hdc1080_readDeviceId(uint16_t *value, struct hdc1080_dev *dev)
{
	uint8_t buf[2];
	
	dev->intf_rslt = hdc1080_get_regs(HDC1080_DEVICE_ID, buf, 2, dev );
	
	*value = ( (uint16_t)buf[0] << 8);
	*value |= ( (uint16_t)buf[1]);
	return dev->intf_rslt;
}

int8_t hdc1080_set_sensor_settings(uint16_t settings, struct hdc1080_dev *dev)
{
    /* Check for null pointer in the device structure*/
    dev->intf_rslt = null_ptr_check(dev);
	
	uint8_t buf[2];
	
	buf[0] = (uint8_t)(settings >> 8);
	buf[1] = (uint8_t)(settings & 0x00FF);
	
    /* Proceed if null check is fine */
    if (dev->intf_rslt == HDC1080_OK)
    {
        dev->intf_rslt = hdc1080_set_regs(HDC1080_CONFIGURATION, buf, 2, dev);
    }

    return dev->intf_rslt;
}

int8_t hdc1080_get_sensor_settings(uint16_t *settings, struct hdc1080_dev *dev)
{
	uint8_t buf[2];
    /* Check for null pointer in the device structure*/
    dev->intf_rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if (dev->intf_rslt == HDC1080_OK)
    {
            dev->intf_rslt = hdc1080_get_regs(HDC1080_CONFIGURATION, buf, 2, dev);
    }
	
	*settings = ( (uint16_t)buf[0] << 8);
	*settings |= ( (uint16_t)buf[1]);
		
    return dev->intf_rslt;
}

int8_t hdc1080_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct hdc1080_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if ((rslt == 0) && (reg_data != NULL))
    {
        /* Read the data  */
        dev->intf_rslt = dev->read(reg_addr, reg_data, len, dev->dev_address);

        /* Check for communication error */
        if (dev->intf_rslt != HDC1080_OK)
        {
            rslt = HDC1080_E_COMM_FAIL;
        }
    }
    else
    {
        rslt = HDC1080_E_NULL_PTR;
    }

    return rslt;
}

int8_t hdc1080_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, struct hdc1080_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Check for arguments validity */
    if ((rslt == HDC1080_OK) && (reg_data != NULL))
    {
        if (len != 0)
        {
            dev->intf_rslt = dev->write(reg_addr, reg_data, len, dev->dev_address);

            /* Check for communication error */
            if (dev->intf_rslt != HDC1080_OK)
            {
                rslt = HDC1080_E_COMM_FAIL;
            }
        }
        else
        {
            rslt = HDC1080_E_INVALID_LEN;
        }
    }
    else
    {
        rslt = HDC1080_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
int8_t null_ptr_check(const struct hdc1080_dev *dev)
{
    int8_t rslt;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL))
    {
        // Device structure pointer is not valid
        rslt = HDC1080_E_NULL_PTR;
    }
    else
    {
        // Device structure is fine
        rslt = HDC1080_OK;
    }
	
    return rslt;
}
