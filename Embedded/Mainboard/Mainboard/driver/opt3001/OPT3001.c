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

#include <math.h>
#include "OPT3001.h"

int16_t opt3001_init(struct opt3001_dev *dev)
{
	uint8_t rslt = OPT3001_NO_ERROR;
	uint16_t val = 0;
	
	rslt = opt3001_readManufacturerID(&val, dev);
	if(val == 0x5449)
	{
		rslt = opt3001_readDeviceID(&val, dev);
		if (val == 0x3001)
		{
			static OPT3001_Config newConfig;
			
			newConfig.RangeNumber = 0x0C;
			newConfig.ConvertionTime = 0x01;
			newConfig.Latch = 0x1;
			newConfig.ModeOfConversionOperation = 0x03;
			
			rslt = 0; //opt3001_writeConfig(newConfig, dev);
			
			if (rslt != OPT3001_NO_ERROR)
			{
				rslt = OPT3001_COMM_FAIL;
			}
			else
			{
				rslt = opt3001_readConfig(&newConfig, dev);
			}
		}
		else
		{
			rslt = OPT3001_DEV_NOT_FOUND;
		}
	}
	else
	{
		rslt = OPT3001_DEV_NOT_FOUND;
	}

	return rslt;
}

int16_t opt3001_readManufacturerID(uint16_t *value, struct opt3001_dev *dev)
{
	uint8_t buf[2];
		
	dev->intf_rslt = opt3001_get_regs(MANUFACTURER_ID, buf, 2, dev );
		
	*value = ( (uint16_t)buf[0] << 8);
	*value |= ( (uint16_t)buf[1]);
		
	return dev->intf_rslt;
}

int16_t opt3001_readDeviceID(uint16_t *value, struct opt3001_dev *dev)
{
	uint8_t buf[2];
	
	dev->intf_rslt = opt3001_get_regs(DEVICE_ID, buf, 2, dev );
	
	*value = ( (uint16_t)buf[0] << 8);
	*value |= ( (uint16_t)buf[1]);
	return dev->intf_rslt;
}

int16_t opt3001_readConfig(OPT3001_Config *config, struct opt3001_dev *dev)
{
	uint8_t buf[2];
	
	dev->intf_rslt = opt3001_get_regs(CONFIG, buf, 2, dev );
		
	config->rawData = ( (uint16_t)buf[0] << 8);
	config->rawData |= ( (uint16_t)buf[1]);
	return dev->intf_rslt;
}

int16_t opt3001_writeConfig(OPT3001_Config config, struct opt3001_dev *dev)
{
    /* Check for null pointer in the device structure*/
    dev->intf_rslt = opt3001_null_ptr_check(dev);
    
    uint8_t buf[2];
    
    buf[0] = (uint8_t)(config.rawData >> 8);
    buf[1] = (uint8_t)(config.rawData & 0x00FF);
    
    /* Proceed if null check is fine */
    if (dev->intf_rslt == OPT3001_NO_ERROR)
    {
	    dev->intf_rslt = opt3001_set_regs(CONFIG, buf, 2, dev);
    }

    return dev->intf_rslt;
}

int16_t opt3001_readResult(struct OPT3001_Value *value, struct opt3001_dev *dev)
{
	return opt3001_readRegister(RESULT, value, dev);
}


int16_t opt3001_readHighLimit(struct OPT3001_Value *value, struct opt3001_dev *dev)
{
	return opt3001_readRegister(HIGH_LIMIT, value, dev);
}

int16_t opt3001_readLowLimit(struct OPT3001_Value *value, struct opt3001_dev *dev)
{
	return opt3001_readRegister(LOW_LIMIT, value, dev);
}

int16_t opt3001_readRegister(OPT3001_Commands command, struct OPT3001_Value *value, struct opt3001_dev *dev)
{
	uint8_t buf[2];
	OPT3001_ER er;
	OPT3001_ErrorCode error = OPT3001_NO_ERROR;
	
	error = opt3001_get_regs(command, buf, 2, dev);
	
	if (error == OPT3001_NO_ERROR)
	{
		er.rawData = ( (uint16_t)buf[0] << 8);
		er.rawData |= ( (uint16_t)buf[1]);
				
		value->error = OPT3001_NO_ERROR;
		value->raw = er;
		value->lux = 0.01 * pow(2, er.Exponent)*er.Result;
	}
	else
	{
		value->error = error;
	}

	return value->error;
}

int8_t opt3001_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct opt3001_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = opt3001_null_ptr_check(dev);

    /* Proceed if null check is fine */
    if ((rslt == 0) && (reg_data != NULL))
    {
        /* Read the data  */
        dev->intf_rslt = dev->read(reg_addr, reg_data, len, dev->dev_address);

        /* Check for communication error */
        if (dev->intf_rslt != OPT3001_NO_ERROR)
        {
            rslt = OPT3001_COMM_FAIL;
        }
    }
    else
    {
        rslt = OPT3001_NULL_PTR;
    }

    return rslt;
}

int8_t opt3001_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, struct opt3001_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = opt3001_null_ptr_check(dev);

    /* Check for arguments validity */
    if ((rslt == OPT3001_NO_ERROR) && (reg_data != NULL))
    {
        if (len != 0)
        {
            dev->intf_rslt = dev->write(reg_addr, reg_data, len, dev->dev_address);

            /* Check for communication error */
            if (dev->intf_rslt != OPT3001_NO_ERROR)
            {
                rslt = OPT3001_COMM_FAIL;
            }
        }
        else
        {
            rslt = OPT3001_INVALID_LEN;
        }
    }
    else
    {
        rslt = OPT3001_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
int8_t opt3001_null_ptr_check(const struct opt3001_dev *dev)
{
    int8_t rslt;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL))
    {
        /* Device structure pointer is not valid */
        rslt = OPT3001_NULL_PTR;
    }
    else
    {
        /* Device structure is fine */
        rslt = OPT3001_NO_ERROR;
    }

    return rslt;
}
