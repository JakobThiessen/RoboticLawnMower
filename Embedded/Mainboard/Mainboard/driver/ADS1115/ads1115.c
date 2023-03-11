/*
 * ads1115.c
 *
 * Created: 14.06.2018 
 * Author: MiBekel
 */  

#include "ads1115.h"

/* CONVERSION register address */
#define CONVERSION_ADDRESS                     ((uint16_t) 0x00)

/* CONFIG register address */
#define CONFIG_ADDRESS                         ((uint16_t) 0x01)
#define CONFIG_DEFAULT                         ((uint16_t) 0x8583)
/* CONFIG register field masks */
#define CONFIG_OS_MASK                         ((uint16_t) 0x8000)
#define CONFIG_MUX_MASK                        ((uint16_t) 0x7000)
#define CONFIG_PGA_MASK                        ((uint16_t) 0x0E00)
#define CONFIG_MODE_MASK                       ((uint16_t) 0x0100)
#define CONFIG_DR_MASK                         ((uint16_t) 0x00E0)
#define CONFIG_COMP_MODE_MASK                  ((uint16_t) 0x0010)
#define CONFIG_COMP_POL_MASK                   ((uint16_t) 0x0008)
#define CONFIG_COMP_LAT_MASK                   ((uint16_t) 0x0004)
#define CONFIG_COMP_QUE_MASK                   ((uint16_t) 0x0003)

/* OS field values */
#define CONFIG_OS_CONVERTING                   ((uint16_t) 0x0000)
#define CONFIG_OS_CONV_START                   ((uint16_t) 0x8000)

/* MUX field values */
#define CONFIG_MUX_AIN0_AIN1                   ((uint16_t) 0x0000)
#define CONFIG_MUX_AIN0_AIN3                   ((uint16_t) 0x1000)
#define CONFIG_MUX_AIN1_AIN3                   ((uint16_t) 0x2000)
#define CONFIG_MUX_AIN2_AIN3                   ((uint16_t) 0x3000)
#define CONFIG_MUX_AIN0_GND                    ((uint16_t) 0x4000)
#define CONFIG_MUX_AIN1_GND                    ((uint16_t) 0x5000)
#define CONFIG_MUX_AIN2_GND                    ((uint16_t) 0x6000)
#define CONFIG_MUX_AIN3_GND                    ((uint16_t) 0x7000)

/* PGA field values */
#define CONFIG_PGA_6p144V                      ((uint16_t) 0x0000)
#define CONFIG_PGA_4p096V                      ((uint16_t) 0x0200)
#define CONFIG_PGA_2p048V                      ((uint16_t) 0x0400)
#define CONFIG_PGA_1p024V                      ((uint16_t) 0x0600)
#define CONFIG_PGA_0p512V                      ((uint16_t) 0x0800)
#define CONFIG_PGA_0p256V                      ((uint16_t) 0x0A00)

/* MODE field values */
#define CONFIG_MODE_CONT                       ((uint16_t) 0x0000)
#define CONFIG_MODE_SS                         ((uint16_t) 0x0100)

/* DR field values */
#define CONFIG_DR_8SPS                         ((uint16_t) 0x0000)
#define CONFIG_DR_16SPS                        ((uint16_t) 0x0020)
#define CONFIG_DR_32SPS                        ((uint16_t) 0x0040)
#define CONFIG_DR_64SPS                        ((uint16_t) 0x0060)
#define CONFIG_DR_128SPS                       ((uint16_t) 0x0080)
#define CONFIG_DR_250SPS                       ((uint16_t) 0x00A0)
#define CONFIG_DR_475SPS                       ((uint16_t) 0x00C0)
#define CONFIG_DR_860SPS                       ((uint16_t) 0x00E0)

/* COMP_MODE field values */
#define CONFIG_COMP_MODE_TRAD                  ((uint16_t) 0x0000)
#define CONFIG_COMP_MODE_WINDOW                ((uint16_t) 0x0010)

/* COMP_POL field values */
#define CONFIG_COMP_POL_ACTIVE_LO              ((uint16_t) 0x0000)
#define CONFIG_COMP_POL_ACTIVE_HI              ((uint16_t) 0x0008)

/* COMP_LAT field values */
#define CONFIG_COMP_LAT_NON_LATCH              ((uint16_t) 0x0000)
#define CONFIG_COMP_LAT_LATCH                  ((uint16_t) 0x0004)

/* COMP_QUE field values */
#define CONFIG_COMP_QUE_ASSERT1                ((uint16_t) 0x0000)
#define CONFIG_COMP_QUE_ASSERT2                ((uint16_t) 0x0001)
#define CONFIG_COMP_QUE_ASSERT4                ((uint16_t) 0x0002)
#define CONFIG_COMP_QUE_DISABLE                ((uint16_t) 0x0003)




/* LO_THRESH register address */
#define LO_THRESH_ADDRESS                      ((uint16_t) 0x02)

/* LO_THRESH default (reset) value */
#define LO_THRESH_DEFAULT                      ((uint16_t) 0x8000)

/* LO_THRESH register field masks */
#define LO_THRESH_LO_THRESHOLD_MASK            ((uint16_t) 0xFFFF)



/* HI_THRESH register address */
#define HI_THRESH_ADDRESS                      ((uint16_t) 0x03)

/* HI_THRESH default (reset) value */
#define HI_THRESH_DEFAULT                      ((uint16_t) 0x7FFF)

/* HI_THRESH register field masks */
#define HI_THRESH_HI_THRESHOLD_MASK            ((uint16_t) 0xFFFF)




// Initialize the driver instance and set the device in power down mode.
bool ads1115_Open(ads1115_t* handle, uint8_t iicAddress, iic_Read_t fRead, iic_Write_t fWrite)
{	
	handle->DevAddr = iicAddress;
	handle->Read = fRead;
	handle->Write = fWrite;
	
	uint8_t dummy = 0;
	return (0 == handle->Read(handle->DevAddr, &dummy, 1));
}


// Converts in single shot mode with power down and 8 SPS. The call is blocking until the conversion
// has finished and the result was read. The comparator is disabled.
int16_t ads1115_ConvertOnce(ads1115_t* handle, uint8_t muxConfig, uint8_t pga)
{
	uint8_t buffer[3];
	
	// Initiate a conversion
	buffer[0] = CONFIG_ADDRESS;
	buffer[1] = ((CONFIG_OS_CONV_START | CONFIG_MODE_SS) >> 8) | muxConfig | pga; // start a single conversion
	buffer[2] = CONFIG_DR_8SPS | CONFIG_COMP_QUE_DISABLE; // sample rate: 8 Hz, comparator disabled and high-z
	
	uint8_t i2cResult = handle->Write(handle->DevAddr, buffer, 3);
	if (i2cResult != 0)
	{
		 return 0;
	}
	
	// Wait until the conversion has finished
	// Poll for end of conversion
	buffer[0] = CONFIG_ADDRESS;
	buffer[1] = 0;
	do
	{
		handle->Write(handle->DevAddr, buffer, 1);
		handle->Read(handle->DevAddr, &buffer[1], 1);
	} while ((buffer[1] & 0x80) == 0);

	
	// Read the conversion result
	buffer[0] = CONVERSION_ADDRESS;
	handle->Write(handle->DevAddr, buffer, 1);
	handle->Read(handle->DevAddr, buffer, 2);
	
	int16_t value_s16 = (int16_t)buffer[0] << 8 | buffer[1];
	
	return value_s16;
}

