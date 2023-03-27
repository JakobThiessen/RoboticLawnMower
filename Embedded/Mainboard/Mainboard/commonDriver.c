/*
 * commonDriver.c
 *
 * Created: 24.03.2023 19:07:52
 *  Author: jakob
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <string.h>

#include "commonDriver.h"

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;

	I2C_0_sendData(dev_addr, &reg_addr, 1);
	rslt = I2C_0_getData(dev_addr, (uint8_t *)reg_data, (uint8_t)len);

	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	uint8_t data[100];
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;

	data[0] = reg_addr;
	memcpy(&data[1], reg_data, len);

	rslt = I2C_0_sendData(dev_addr, (uint8_t*)&data, len + 1);

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
int8_t user_i2c_1_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	uint8_t data[100];
	int8_t rslt = 0;
	uint8_t dev_addr = *(uint8_t*)intf_ptr;
	
	data[0] = reg_addr;
	memcpy(&data[1], reg_data, len);
	
	rslt = I2C_1_sendData(dev_addr, (uint8_t*)&data, len + 1);
	
	return rslt;
}

int8_t user_i2c_1_read_pca(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	int8_t rslt = 0;
	
	rslt = I2C_1_sendData(dev_addr, &reg_addr, 1);
	rslt += I2C_1_getData(dev_addr, data, (uint8_t)len);

	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_1_write_pca(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	uint8_t buffer[250];
	int8_t rslt = 0;

	buffer[0] = reg_addr;
	memcpy(&buffer[1], data, len);
	
	rslt = I2C_1_sendData(dev_addr, (uint8_t*)&buffer, len + 1);

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
	uint8_t buffer[250];
	int8_t rslt = 0;

	buffer[0] = reg_addr;
	memcpy(&buffer[1], data, len);
	
	rslt = I2C_0_sendData(dev_addr, (uint8_t*)&buffer, len + 1);
	//rslt = I2C_1_sendData(dev_addr, &reg_addr, 1);
	//rslt = I2C_1_sendData(dev_addr, data, len);
	
	return rslt;
}


int8_t user_i2c_read_ads1115(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
	int8_t rslt = 0;

	rslt = I2C_0_getData(dev_addr, data, len);

	return rslt;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write_ads1115(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
	int8_t rslt = 0;
	rslt = I2C_0_sendData(dev_addr, data, len);

	return rslt;
}

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period_us, void *intf_ptr)
{
	vTaskDelay(pdMS_TO_TICKS(1));
/*
	for(int p = 0; p < period_us; p++)
	{
		_delay_us(1);
	}
*/
}

void user_delay_ms(uint32_t period_ms)
{
	vTaskDelay(pdMS_TO_TICKS(period_ms));
/*
	for(int p = 0; p < period_ms; p++)
	{
		_delay_ms(1);
	}
*/
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