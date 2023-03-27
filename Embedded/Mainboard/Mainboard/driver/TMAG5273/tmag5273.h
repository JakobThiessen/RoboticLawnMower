/*!
 *  @file PCA9685
 *
 */
 
#ifndef _TMAG5273_H
#define _TMAG5273_H

#define TMAG5273A1_I2C_DEF_ADDR			0x35
#define TMAG5273B1_I2C_DEF_ADDR			0x22
#define TMAG5273C1_I2C_DEF_ADDR			0x78
#define TMAG5273D1_I2C_DEF_ADDR			0x44
#define TMAG5273A2_I2C_DEF_ADDR			0x35
#define TMAG5273B2_I2C_DEF_ADDR			0x22
#define TMAG5273C2_I2C_DEF_ADDR			0x78
#define TMAG5273D2_I2C_DEF_ADDR			0x44


#define TMAG5273_DEVICE_CONFIG_1		0x00
#define TMAG5273_DEVICE_CONFIG_2		0x01
#define TMAG5273_SENSOR_CONFIG_1		0x02
#define TMAG5273_SENSOR_CONFIG_2		0x03
	/* SENSOR_CONFIG_2 register field masks */
	#define SENSOR_CONFIG_2_X_Y_RANGE_MASK			((uint8_t) 0x02)
	#define SENSOR_CONFIG_2_Z_RANGE_MASK			((uint8_t) 0x01)
	
#define TMAG5273_X_THR_CONFIG			0x04
#define TMAG5273_Y_THR_CONFIG			0x05
#define TMAG5273_Z_THR_CONFIG			0x06
#define TMAG5273_T_CONFIG				0x07
#define TMAG5273_INT_CONFIG_1			0x08
#define TMAG5273_MAG_GAIN_CONFIG		0x09
#define TMAG5273_MAG_OFFSET_CONFIG_1	0x0A
#define TMAG5273_MAG_OFFSET_CONFIG_2	0x0B
#define TMAG5273_I2C_ADDRESS			0x0C
#define TMAG5273_DEVICE_ID				0x0D
	/* DEVICE_ID default (reset) value */
	#define DEVICE_ID_DEFAULT						((uint8_t) 0x01)
	/* DEVICE_ID register field masks */
	#define DEVICE_ID_VER_MASK						((uint8_t) 0x03)
	/* VER field values */
	#define DEVICE_ID_VER_40mTand80mTRangeOption0	((uint8_t) 0x00)
	#define DEVICE_ID_VER_40mTand80mTRange			((uint8_t) 0x01)
	#define DEVICE_ID_VER_133mTand266mTRange		((uint8_t) 0x02)
	#define DEVICE_ID_VER_Reserved1					((uint8_t) 0x03)
		
#define TMAG5273_MANUFACTURER_ID_LSB	0x0E
#define TMAG5273_MANUFACTURER_ID_MSB	0x0F
#define TMAG5273_T_MSB_RESULT			0x10
#define TMAG5273_T_LSB_RESULT			0x11
#define TMAG5273_X_MSB_RESULT			0x12
#define TMAG5273_X_LSB_RESULT			0x13
#define TMAG5273_Y_MSB_RESULT			0x14
#define TMAG5273_Y_LSB_RESULT			0x15
#define TMAG5273_Z_MSB_RESULT			0x16
#define TMAG5273_Z_LSB_RESULT			0x17
#define TMAG5273_CONV_STATUS			0x18
#define TMAG5273_ANGLE_RESULT_MSB		0x19
#define TMAG5273_ANGLE_RESULT_LSB		0x1A
#define TMAG5273_MAGNITUDE_RESULT		0x1B
#define TMAG5273_DEVICE_STATUS			0x1C

#define TMAG5273_DEVICE_ID_A1			0x01		// ±40-mT and ±80-mT range
#define TMAG5273_DEVICE_ID_A2			0x02		// ±133-mT and ±266-mT range
#define TMAG5273_MANUFACTURER_ID		0x5449

/*****************************************************************************/
/* type definitions */

typedef int8_t (*tmag5273_read_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*tmag5273_write_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef void (*tmag5273_delay_fptr_t)(uint32_t period);


/*
8-bit, 2's complement X axis threshold code for limit
check. The range of possible threshold entrees can be
+/-128. The threshold value in mT is calculated for
A1 as (40(1+X_Y_RANGE)/128)*X_THR_CONFIG, for A2 as
(133(1+X_Y_RANGE)/128)*X_THR_CONFIG. Default 0h means no
threshold comparison.
*/

struct tmag5273_XYZ_threshold
{
	int8_t x;	// can be +/-128	A1 as (40(1+X_Y_RANGE)/128)*X_THR_CONFIG	A2 as (133(1+X_Y_RANGE)/128)*X_THR_CONFIG
	int8_t y;	// can be +/-128	A1 as (40(1+X_Y_RANGE)/128)*X_THR_CONFIG	A2 as (133(1+X_Y_RANGE)/128)*X_THR_CONFIG
	int8_t z;	// can be +/-128	A1 as (40(1+X_Y_RANGE)/128)*X_THR_CONFIG	A2 as (133(1+X_Y_RANGE)/128)*X_THR_CONFIG
};

/*!
 * @brief tmag5273 sensor data structure
 */
struct tmag5273_raw_sensor_data
{
    /*! X-axis sensor data */
    int16_t x;

    /*! Y-axis sensor data */
    int16_t y;

    /*! Z-axis sensor data */
    int16_t z;
};

/*!
 * @brief tmag5273 sensor data structure
 */
struct tmag5273_sensor_data
{
    /*! X-axis sensor data */
    float x;

    /*! Y-axis sensor data */
    float y;

    /*! Z-axis sensor data */
    float z;
};

struct tmag5273_dev
{
	/*! Device Id */
	uint8_t deviceID;
	
	/*! MANUFACTURER ID */
	uint16_t manufacturerID;
	
	/*! Device I2C Address */
	uint8_t i2c_addr;

	/*! result of read/write function*/
	int8_t rslt;

	/*! Read function pointer */
	tmag5273_read_fptr_t read;

	/*! Write function pointer */
	tmag5273_write_fptr_t write;

	/*!  Delay function pointer */
	tmag5273_delay_fptr_t delay_ms;
	
	/*!  Configuration Data */
	uint16_t rangeXY;
	uint16_t rangeZ;
};

int8_t tmag5273_init(struct tmag5273_dev *dev);
int8_t tmag5273_setDeviceConfig(uint16_t config, struct tmag5273_dev *dev);
int8_t tmag5273_setSensorConfig(uint16_t config, struct tmag5273_dev *dev);
int8_t tmag5273_setXYZ_ThrConfig(struct tmag5273_XYZ_threshold thr, struct tmag5273_dev *dev);
int8_t tmag5273_setTempConfig(uint8_t config, struct tmag5273_dev *dev);

int8_t tmag5273_getDeviceID(uint8_t *devID, struct tmag5273_dev *dev);
int8_t tmag5273_getManufacturerID(uint16_t *manID, struct tmag5273_dev *dev);

int8_t readTemperatureData(float *temp, struct tmag5273_dev *dev);

int8_t readRawXYZData(struct tmag5273_raw_sensor_data *vector, struct tmag5273_dev *dev);
int8_t readXYZData(struct tmag5273_sensor_data *vector, struct tmag5273_dev *dev);

int8_t readRawAngleData(uint16_t *angle, struct tmag5273_dev *dev);
int8_t readAngleData(float *angle, struct tmag5273_dev *dev);

int8_t readRawMagnitudeData(uint8_t *magnitude, struct tmag5273_dev *dev);

//******************//
// Helper Functions //
//******************//
int8_t TMAG5x73getXYZrange(uint16_t *rangeXY, uint16_t *rangeZ, struct tmag5273_dev *dev);
int8_t TMAG5x73getVersion(uint8_t *version, struct tmag5273_dev *dev);

#endif