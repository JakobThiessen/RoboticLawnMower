/*


*/

#ifndef _ADS1115_h
#define _ADS1115_h

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* type definitions */
typedef int8_t (*ads115_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint16_t devAddr);
typedef int8_t (*ads115_write_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, uint16_t devAddr);
typedef void (*ads115_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

#define ADS1115_IIC_ADDRESS_0			0x48
#define ADS1115_IIC_ADDRESS_1			0x49
#define ADS1115_IIC_ADDRESS_2			0x4A
#define ADS1115_IIC_ADDRESS_3			0x4B

#define ADS1115_ADR_POINTER_REG			0x00
#define ADS1115_ADR_POINTER_REG_MASK	0x03

#define	ADS1115_CONV_POINTER			0x00
#define	ADS1115_CONF_POINTER			0x01
#define	ADS1115_THRES_L_POINTER			0x02
#define	ADS1115_THRES_L_POINTER			0x03

/*! @name OPERATIONAL STATUS DEFINITIONS  */
#define ADS1115_OS_POS					UINT8_C(15)
#define ADS1115_OS_NO_EFFECT			UINT8_C(0)
#define ADS1115_OS_START_CONV			UINT8_C(1)

/*! @name INPUT MULTIPLEXER DEFINITIONS  */
#define ADS1115_MUX_POS					UINT8_C(12)
#define ADS1115_MUX_AIN0_AIN1			UINT8_C(0x00)
#define ADS1115_MUX_AIN0_AIN3			UINT8_C(0x01)
#define ADS1115_MUX_AIN1_AIN3			UINT8_C(0x02)
#define ADS1115_MUX_AIN2_AIN3			UINT8_C(0x03)
#define ADS1115_MUX_AIN0_GND			UINT8_C(0x04)
#define ADS1115_MUX_AIN1_GND			UINT8_C(0x05)
#define ADS1115_MUX_AIN2_GND			UINT8_C(0x06)
#define ADS1115_MUX_AIN3_GND			UINT8_C(0x07)

/*! @name PGA DEFINITIONS  */
#define ADS1115_PGA_POS					UINT8_C(9)
#define ADS1115_PGA_6144_mV				UINT8_C(0x00)
#define ADS1115_PGA_4096_mV				UINT8_C(0x01)
#define ADS1115_PGA_2048_mV				UINT8_C(0x02)
#define ADS1115_PGA_1024_mV				UINT8_C(0x03)
#define ADS1115_PGA_0512_mV				UINT8_C(0x04)
#define ADS1115_PGA_0256_mV				UINT8_C(0x05)
#define ADS1115_PGA_0256_mV				UINT8_C(0x06)
#define ADS1115_PGA_0256_mV				UINT8_C(0x07)

/*! @name MODE DEFINITIONS  */
#define ADS1115_MODE_POS				UINT8_C(8)
#define ADS1115_MODE_CONTINUOUS			UINT8_C(0)
#define ADS1115_MODE_SINGLE_SHOT		UINT8_C(1)

/*! @name DATA RATE DEFINITIONS  */
#define ADS1115_DATA_RATE_POS			UINT8_C(5)
#define ADS1115_DATA_RATE_08SPS			UINT8_C(0x00)
#define ADS1115_DATA_RATE_16SPS			UINT8_C(0x01)
#define ADS1115_DATA_RATE_32SPS			UINT8_C(0x02)
#define ADS1115_DATA_RATE_64SPS			UINT8_C(0x03)
#define ADS1115_DATA_RATE_128SPS		UINT8_C(0x04)
#define ADS1115_DATA_RATE_250SPS		UINT8_C(0x05)
#define ADS1115_DATA_RATE_475SPS		UINT8_C(0x06)
#define ADS1115_DATA_RATE_860SPS		UINT8_C(0x07)

/*! @name COMPARATOR MODE DEFINITIONS  */
#define ADS1115_COMP_MODE_POS			UINT8_C(4)
#define ADS1115_COMP_MODE_TARDITIONAL	UINT8_C(0)
#define ADS1115_COMP_MODE_WINDOW		UINT8_C(1)

/*! @name COMPARATOR POLARITY DEFINITIONS  */
#define ADS1115_COMP_POL_POS			UINT8_C(3)
#define ADS1115_COMP_POL_ACTIVE_LOW		UINT8_C(0)
#define ADS1115_COMP_POL_ACTIVE_HIGH	UINT8_C(1)

/*! @name LATCHING COMPARATOR DEFINITIONS  */
#define ADS1115_COMP_LAT_POS			UINT8_C(2)
#define ADS1115_COMP_LAT_NON			UINT8_C(0)
#define ADS1115_COMP_LAT				UINT8_C(1)

/*! @name COMPARATOR QUEUE DEFINITIONS  */
#define ADS1115_COMP_QUE_POS			UINT8_C(0)
#define ADS1115_COMP_QUE_AFTER_ONE		UINT8_C(0)
#define ADS1115_COMP_QUE_AFTER_TWO		UINT8_C(1)
#define ADS1115_COMP_QUE_AFTER_FOUR		UINT8_C(2)
#define ADS1115_COMP_QUE_DISABLE		UINT8_C(3)

/**\name API success code */
#define ADS1115_OK					(int8_t)(0)

/**\name API error codes */
#define ADS1115_E_NULL_PTR			(int8_t)(-1)
#define ADS1115_E_DEV_NOT_FOUND		(int8_t)(-2)
#define ADS1115_E_INVALID_LEN		(int8_t)(-3)
#define ADS1115_E_COMM_FAIL			(int8_t)(-4)
#define ADS1115_E_SLEEP_MODE_FAIL	(int8_t)(-5)
#define ADS1115_E_NVM_COPY_FAILED	(int8_t)(-6)

#define ADS1115_CONFIG_RST			(1UL << (14))
#define ADS1115_CONFIG_HEAT_EN		(1UL << (13))
#define ADS1115_CONFIG_MODE_EN		(1UL << (12))		


struct ads115_dev
{
    /*! Device Address */
    uint8_t dev_address;

    /*< Read function pointer */
    ads115_read_fptr_t read;

    /*< Write function pointer */
    ads115_write_fptr_t write;

    /*< Delay function pointer */
    ads115_delay_us_fptr_t delay_us;

    /*< Sensor settings */
    uint16_t settings;

    /*< Variable to store result of read/write function */
    int8_t intf_rslt;
};

int8_t null_ptr_check(const struct ads115_dev *dev);

int8_t ads115_init(struct ads115_dev *dev);

int8_t ads115_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, struct ads115_dev *dev);
int8_t ads115_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct ads115_dev *dev);

int8_t ads115_readManufacturerId(uint16_t *value, struct ads115_dev *dev);		// 0x5449 ID of Texas Instruments
int8_t ads115_readDeviceId(uint16_t *value, struct ads115_dev *dev);			// 0x1050 ID of the device

int8_t ads115_triggerMeasurement( struct ads115_dev *dev);

int8_t ads115_readTemperature(float *value, struct ads115_dev *dev);
int8_t ads115_readHumidity(float *value, struct ads115_dev *dev);

#endif