/*
 * commonDataMgmt.h
 *
 * Created: 01.08.2023 18:47:28
 *  Author: jakob
 */ 

#ifndef COMMONDATAMGMT_H_
#define COMMONDATAMGMT_H_


extern struct globalData
{
	int16_t temperature;		// [2]
	int16_t pressure;			// [2]
	int16_t humidity;			// [2]
	
	int16_t dataGyro[3];		// [6]
	int16_t dataAcc[3];			// [6]
	int16_t dataCompass[3];		// [6]

	int16_t analogSensor[8];	// [16]
			
	int16_t magnetSensor[3];	// [6]
	
	uint16_t motorVoltage[5];	// [10]
	int16_t motorCurrent[5];	// [10]
	
	int16_t	velocity[5];		// [10]
	
	float	latitude;			// [4]
	float	longitude;			// [4]
	uint8_t hours;				// [1]
	uint8_t minutes;				// [1]
	uint8_t seconds;				// [1]
};

struct globalData glbRoboterData;

void initCommonDataMgmt();
int8_t getGlbRoboterData(uint8_t *buffer, uint8_t *size);

#endif /* COMMONDATAMGMT_H_ */