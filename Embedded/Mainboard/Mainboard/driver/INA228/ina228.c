/*
 * INA228 - TI Current/Voltage/Power Monitor Code
 * Copyright (C) 2021 Craig Peacock
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ina228.h"

/*
 * SHUNT_CAL is a conversion constant that represents the shunt resistance
 * used to calculate current value in Amps. This also sets the resolution
 * (CURRENT_LSB) for the current register.
 *
 * SHUNT_CAL is 15 bits wide (0 - 32768)
 *
 * SHUNT_CAL = 13107.2 x 10^6 x CURRENT_LSB x Rshunt
 *
 * CURRENT_LSB = Max Expected Current / 2^19
 */

#define CURRENT_LSB 	0.0000190735
#define SHUNT_CAL		2500

static volatile float currentShuntLsb = 0;


int8_t ina228_init(struct ina228_dev *dev)
{
	dev->isConfigured = false;
	//i2c_write_short(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_CONFIG, 0x8000);	// Reset
	dev->intf_rslt = dev->write(INA228_CONFIG, 0x8000, 2, dev->intf_ptr);
	dev->intf_rslt += dev->read(INA228_MANUFACTURER_ID, &dev->manID, 1, dev->intf_ptr);
	dev->intf_rslt += dev->read(INA228_DEVICE_ID, &dev->devID, 1, dev->intf_ptr);

	//i2c_write_short(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_SHUNT_CAL, SHUNT_CAL);
	dev->intf_rslt += dev->write(INA228_SHUNT_CAL, SHUNT_CAL, 2, dev->intf_ptr);
	
	if(dev->shunt_ADCRange == 0)	currentShuntLsb = CURRENT_SHUNT_LSB_ADCRange_0;
	if(dev->shunt_ADCRange == 1)	currentShuntLsb = CURRENT_SHUNT_LSB_ADCRange_1;
	
	if(dev->intf_rslt == 0)
	{
		dev->isConfigured = true;
	}
	
	return dev->intf_rslt;
}

int8_t ina228_voltage(float *value, struct ina228_dev *dev)
{
	uint8_t data[3];
	int32_t iBusVoltage;
	float fBusVoltage;
	bool sign;

	//i2c_read_buf(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_VBUS, (uint8_t *)&iBusVoltage, 3);
	dev->read(INA228_VBUS, (uint8_t *)&data, 3, dev->intf_ptr);
	sign = (data[0] & 0x80) == 0x80;
	
	iBusVoltage = (int32_t)(((int32_t)data[0] << 16 | (int32_t)data[1] << 8 | (int32_t)data[2]) >> 4 );
	iBusVoltage = iBusVoltage & 0xFFFFFF;
	
	if (sign)
	{
		iBusVoltage += 0xFFF00000;
	}
	
	fBusVoltage = (float)(iBusVoltage) * BUS_VOLTAGE_LSB;		// Output in mV

	*value = fBusVoltage;
		
	return dev->intf_rslt;
}

int8_t ina228_dietemp(float *value, struct ina228_dev *dev)
{
	uint16_t iDieTemp;
	float fDieTemp;

	//iDieTemp = i2c_read_short(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_DIETEMP);
	dev->read(INA228_DIETEMP, (uint8_t *)&iDieTemp, 2, dev->intf_ptr);
	
	fDieTemp = (iDieTemp) * DIE_TEMPERATURE_LSB;
	*value = fDieTemp;
	
	return dev->intf_rslt;
}

int8_t ina228_shuntvoltage(float *value, struct ina228_dev *dev)
{
	uint8_t data[3];
	int32_t iShuntVoltage;
	float fShuntVoltage;
	bool sign;

	//i2c_read_buf(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_VSHUNT, (uint8_t *)&iShuntVoltage, 3);
	dev->read(INA228_VSHUNT, &data, 3, dev->intf_ptr);
	sign = (data[0] & 0x80) == 0x80;
	
	iShuntVoltage = (int32_t)(((int32_t)data[0] << 16 | (int32_t)data[1] << 8 | (int32_t)data[2]) >> 4 );
	iShuntVoltage = iShuntVoltage & 0xFFFFFF;
	
	if (sign)
	{
		iShuntVoltage += 0xFFF00000;
	}
	
	fShuntVoltage = (float)(iShuntVoltage) * currentShuntLsb;		// Output in mV
	*value = fShuntVoltage;
	
	return dev->intf_rslt;
}

int8_t ina228_current(float *value, struct ina228_dev *dev)
{
	uint8_t data[3];
	int32_t iCurrent;
	bool sign;

	//i2c_read_buf(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_CURRENT, (uint8_t *)&iCurrent, 3);
	dev->read(INA228_CURRENT, &data, 3, dev->intf_ptr);	
	sign = (data[0] & 0x80) == 0x80;
	
	iCurrent = (int32_t)(((int32_t)data[0] << 16 | (int32_t)data[1] << 8 | (int32_t)data[2]) >> 4 );
	iCurrent = iCurrent & 0xFFFFFF;
	
	if (sign)
	{
		iCurrent += 0xFFF00000;
	}
	*value = (float)iCurrent;
	
	return dev->intf_rslt;
}

int8_t ina228_power(float *value, struct ina228_dev *dev)
{
	uint8_t data[3];
	uint32_t iPower;
	float fPower;

	//i2c_read_buf(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_POWER, (uint8_t *)&iPower, 3);
	dev->read(INA228_POWER, (uint8_t *)&iPower, 3, dev->intf_ptr);
		
	iPower = (int32_t)(((int32_t)data[0] << 16 | (int32_t)data[1] << 8 | (int32_t)data[2]) >> 4 );
	iPower = iPower & 0xFFFFFF;

	fPower = 3.2 * CURRENT_LSB * iPower;
	
	*value = fPower;
	
	return dev->intf_rslt;
}

/*
 * Returns energy in Joules.
 * 1 Watt = 1 Joule per second
 * 1 W/hr = Joules / 3600
 */

int8_t ina228_energy(float *value, struct ina228_dev *dev)
{
	uint64_t iEnergy;
	float fEnergy;

	//i2c_read_buf(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_ENERGY, (uint8_t *)&iEnergy, 5);
	dev->read(INA228_ENERGY, (uint8_t *)&iEnergy, 5, dev->intf_ptr);
	
	//iEnergy = __bswap64(iEnergy & 0xFFFFFFFFFF) >> 24;

	fEnergy = 16 * 3.2 * CURRENT_LSB * iEnergy;

	*value = fEnergy;
	
	return dev->intf_rslt;
}

/*
 * Returns electric charge in Coulombs.
 * 1 Coulomb = 1 Ampere per second.
 * Hence Amp-Hours (Ah) = Coulombs / 3600
 */

int8_t ina228_charge(float *value, struct ina228_dev *dev)
{
	int64_t iCharge;
	float fCharge;
	bool sign;

	//i2c_read_buf(i2c_master_port, INA228_SLAVE_ADDRESS, INA228_CHARGE, (uint8_t *)&iCharge, 5);
	dev->read(INA228_CHARGE, (uint8_t *)&iCharge, 5, dev->intf_ptr);
	
	sign = iCharge & 0x80;
	//iCharge = __bswap64(iCharge & 0xFFFFFFFFFF) >> 24;
	if (sign) iCharge += 0xFFFFFF0000000000;

	fCharge = CURRENT_LSB * iCharge;
	*value = fCharge;
	
	return dev->intf_rslt;
}