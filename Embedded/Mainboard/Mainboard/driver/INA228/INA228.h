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

#ifndef MAIN_INA228_H_
#define MAIN_INA228_H_

#define INA228_SLAVE_ADDRESS	0x40

#define INA228_CONFIG			0x00
#define INA228_ADC_CONFIG		0x01
#define INA228_SHUNT_CAL		0x02
#define INA228_SHUNT_TEMPCO		0x03
#define INA228_VSHUNT			0x04
#define INA228_VBUS				0x05
#define INA228_DIETEMP			0x06
#define INA228_CURRENT			0x07
#define INA228_POWER			0x08
#define INA228_ENERGY			0x09
#define INA228_CHARGE			0x0A
#define INA228_DIAG_ALRT		0x0B
#define INA228_SOVL				0x0C
#define INA228_SUVL				0x0D
#define INA228_BOVL				0x0E
#define INA228_BUVL				0x0F
#define INA228_TEMP_LIMIT		0x10
#define INA228_PWR_LIMIT		0x11
#define INA228_MANUFACTURER_ID	0x3E
#define INA228_DEVICE_ID		0x3F

#define CURRENT_SHUNT_LSB_ADCRange_0	0.0003125f;        // Output in mV when ADCRange = 0
#define CURRENT_SHUNT_LSB_ADCRange_1	0.000078125f;      // Output in mV when ADCRange = 1
#define BUS_VOLTAGE_LSB					0.0001953125f;
#define DIE_TEMPERATURE_LSB				0.0078125f;

/* type definitions */
typedef int8_t (*ina228_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
typedef int8_t (*ina228_write_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
typedef void (*ina228_delay_us_fptr_t)(uint32_t period);

struct ina228_dev
{
	/*! Device Address */
	uint8_t dev_address;

	/*< Interface function pointer used to enable the device address for I2C */
	void *intf_ptr;

	/*< Read function pointer */
	ina228_read_fptr_t read;

	/*< Write function pointer */
	ina228_write_fptr_t write;

	/*< Delay function pointer */
	ina228_delay_us_fptr_t delay_us;

	/*< Sensor settings */
	uint16_t settings;

	/*< Variable to store result of read/write function */
	int8_t intf_rslt;
	
	uint8_t manID;
	uint8_t devID;
	
	uint8_t shunt_ADCRange;
	
	bool isConfigured;
};

int8_t null_ptr_check(const struct ina228_dev *dev);

int8_t ina228_init(struct ina228_dev *dev);
int8_t ina228_voltage(float *value, struct ina228_dev *dev);
int8_t ina228_dietemp(float *value, struct ina228_dev *dev);
int8_t ina228_shuntvoltage(float *value, struct ina228_dev *dev);
int8_t ina228_current(float *value, struct ina228_dev *dev);
int8_t ina228_power(float *value, struct ina228_dev *dev);
int8_t ina228_energy(float *value, struct ina228_dev *dev);
int8_t ina228_charge(float *value, struct ina228_dev *dev);

#endif