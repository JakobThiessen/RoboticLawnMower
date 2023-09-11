/*
 * commonDriver.h
 *
 * Created: 24.03.2023 19:08:11
 *  Author: jakob
 */ 


#ifndef COMMON_DRIVER_H_
#define COMMON_DRIVER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <string.h>

#include "hal/i2c.h"
#include "hal/spi_basic.h"
#include "hal/port.h"
#include "hal/uart.h"
#include "hal/adc.h"

//#include "hal/timer.h"
#include "hal/clock.h"

#include "OS/freeRTOS/include/FreeRTOS.h"
#include "OS/freeRTOS/include/task.h"

#define LED0	PIN3_bm
#define SW0		PIN2_bm
#define DIO		PIN7_bm

#define PIN_WLAN_EN	PIN0_bm
#define MOTOR_EN_M1	PIN3_bm
#define MOTOR_EN_M5	PIN2_bm

#define SENSOR_COLLISION_PORT PORTD
#define SENSOR_COLLISION_00	PIN6_bp
#define SENSOR_COLLISION_01	PIN5_bp
#define SENSOR_COLLISION_02	PIN4_bp

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

int8_t user_i2c_read_bmi160(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
int8_t user_i2c_write_bmi160(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

int8_t user_i2c_1_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t user_i2c_1_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

int8_t user_i2c_1_read_pca(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
int8_t user_i2c_1_write_pca(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

int8_t user_i2c_read_ads1115(uint8_t dev_addr, uint8_t *data, uint8_t len);
int8_t user_i2c_write_ads1115(uint8_t dev_addr, uint8_t *data, uint8_t len);

void user_delay_us(uint32_t period_us, void *intf_ptr);
void user_delay_ms(uint32_t period_ms);

void configuration_spi(void);
uint8_t write_spi_data(uint8_t data);
uint8_t write_spi_buffer(uint8_t *buffer, uint8_t len);

#endif /* COMMONDRIVER_H_ */