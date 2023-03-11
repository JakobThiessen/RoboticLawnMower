/*
 * ads1115.h
 *
 * Created: 14.06.2018 
 * Author: MiBekel
 */ 


#ifndef __ADS1115_H_
#define __ADS1115_H_

#include <inttypes.h>
#include <stdbool.h>
#include <util/delay.h>
//#include "iic.h"

// Selectable I2C addresses, depending on the hardware configuration of the address pin:
//                                    ADDR pin connection
#define ADS1115_ADDRESS1	0x48   /* GND */
#define ADS1115_ADDRESS2	0x49   /* VDD */
#define ADS1115_ADDRESS3	0x50   /* SDA */
#define ADS1115_ADDRESS4	0x51   /* SCL */

// Input multiplexer configuration. In differential mode AIN0 and AIN2 are positive inputs.
#define ADS1115_MUX_AIN0_AIN1    0x00
#define ADS1115_MUX_AIN0_AIN3    0x10
#define ADS1115_MUX_AIN1_AIN3    0x20
#define ADS1115_MUX_AIN2_AIN3    0x30
#define ADS1115_MUX_AIN0_GND     0x40
#define ADS1115_MUX_AIN1_GND     0x50
#define ADS1115_MUX_AIN2_GND     0x60
#define ADS1115_MUX_AIN3_GND     0x70

/* PGA field values */
#define ADS1115_PGA_6p144V       0x00
#define ADS1115_PGA_4p096V       0x02
#define ADS1115_PGA_2p048V       0x04
#define ADS1115_PGA_1p024V       0x06
#define ADS1115_PGA_0p512V       0x08
#define ADS1115_PGA_0p256V       0x0A

typedef uint8_t (*iic_Read_t)(uint8_t address, uint8_t* buffer, uint8_t length);
typedef uint8_t (*iic_Write_t)(uint8_t address, uint8_t* buffer, uint8_t length);

typedef struct
{
	uint8_t		DevAddr;
	iic_Read_t	Read;
	iic_Write_t	Write;
} ads1115_t;


// Initialize the driver instance and set the device in power down mode.
bool ads1115_Open(ads1115_t* handle, uint8_t iicAddress, iic_Read_t fRead, iic_Write_t fWrite);

// Converts in single shot mode with power down and 8 SPS. The call is blocking until the conversion
// has finished and the result was read. The comparator is disabled.
int16_t ads1115_ConvertOnce(ads1115_t* handle, uint8_t muxConfig, uint8_t pga);

#endif
