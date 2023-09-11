/**
 * @file  vl53l1_platform.h
 * @brief Those platform functions are platform dependent and have to be implemented by the user
 */
 
#ifndef _VL53L1_PLATFORM_H_
#define _VL53L1_PLATFORM_H_

#include "..\core\vl53l1_types.h"
#include "..\core\vl53l1_error_codes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	uint32_t dummy;
} VL53L1_Dev_t;

typedef VL53L1_Dev_t *VL53L1_DEV;



/** @brief VL53L1_WriteMulti() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_WriteMulti(
		uint16_t 			dev,
		uint16_t      index,
		uint8_t      *pdata,
		uint32_t      count);
/** @brief VL53L1_ReadMulti() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_ReadMulti(
		uint16_t 			dev,
		uint16_t      index,
		uint8_t      *pdata,
		uint32_t      count);
/** @brief VL53L1_WrByte() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_WrByte(
		uint16_t dev,
		uint16_t      index,
		uint8_t       data);
/** @brief VL53L1_WrWord() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_WrWord(
		uint16_t dev,
		uint16_t      index,
		uint16_t      data);
/** @brief VL53L1_WrDWord() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_WrDWord(
		uint16_t dev,
		uint16_t      index,
		uint32_t      data);
/** @brief VL53L1_RdByte() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_RdByte(
		uint16_t dev,
		uint16_t      index,
		uint8_t      *pdata);
/** @brief VL53L1_RdWord() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_RdWord(
		uint16_t dev,
		uint16_t      index,
		uint16_t     *pdata);
/** @brief VL53L1_RdDWord() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_RdDWord(
		uint16_t dev,
		uint16_t      index,
		uint32_t     *pdata);
/** @brief VL53L1_WaitMs() definition.\n
 * To be implemented by the developer
 */
int8_t VL53L1_WaitMs(
		uint16_t dev,
		int32_t       wait_ms);


/******************************************************************************/
/*! @name           Function Pointers                             */
/******************************************************************************/
#ifndef VL53L1_INTF_RET_TYPE
#define VL53L1_INTF_RET_TYPE     int8_t
#endif

#ifndef VL53L1_INTF_RET_SUCCESS
#define VL53L1_INTF_RET_SUCCESS  int8_t
#endif

typedef int8_t (*vl53l1_iic_Read_t)(uint8_t address, uint8_t* buffer, uint8_t length);
typedef int8_t (*vl53l1_iic_Write_t)(uint8_t address, uint8_t* buffer, uint8_t length);
typedef void (*vl53l1_delay_us_fptr_t)(uint32_t period);

/*!
 * @brief vl53l1 device structure
 */
struct vl53l1_dev
{
    /*! Chip Id */
    uint8_t chip_id;

    /*! Variable that holds result of read/write function */
    VL53L1_INTF_RET_SUCCESS intf_rslt;

    /*! Bus read function pointer */
    vl53l1_iic_Read_t read;

    /*! Bus write function pointer */
    vl53l1_iic_Write_t write;

    /*! delay(in us) function pointer */
    vl53l1_delay_us_fptr_t delay_us;
};

int8_t VL53L1_initInterface(uint8_t chip_id, vl53l1_iic_Read_t read, vl53l1_iic_Write_t write);

#ifdef __cplusplus
}
#endif

#endif
