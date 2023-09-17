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
	int16_t inclination[3];		// [6]
	int16_t dataCompass[3];		// [6]
	uint32_t CompassDeg;		// [4]
	
	int16_t analogSensor[8];	// [16]
			
	int16_t magnetSensor[3];	// [6]
	
	uint16_t motorVoltage[5];	// [10]
	int16_t motorCurrent[5];	// [10]
	
	int16_t	velocity[5];		// [10]
	
	uint8_t gpsValid;			// [1]
	float	latitude;			// [4]
	float	longitude;			// [4]
	
	float speed;				// [4] /*!< Current spead over the ground in knots */
	float course;				// [4] /*!< Current course over ground */
	float variation;			// [4]
						
	uint8_t hours;				// [1]
	uint8_t minutes;			// [1]
	uint8_t seconds;			// [1]
	
	uint8_t date;				// [1]
	uint8_t month;				// [1]
	uint8_t year;				// [1]

	int8_t errorNeigung;		// [1]
	uint8_t sens_collosion_00;	// [1]
	uint8_t sens_collosion_01;	// [1]
	uint8_t sens_collosion_02;	// [1]
	
	uint16_t sens_rain;			// [2]
	
	uint16_t sens_dist_00;		// [2]
	uint16_t sens_dist_01;		// [2]
	uint16_t sens_dist_02;		// [2]
	
	uint16_t state_MotionStateMachine; // [2]
};

struct globalData glbRoboterData;

void initCommonDataMgmt();
int8_t getGlbRoboterData(uint8_t *buffer, uint8_t *size);

#endif /* COMMONDATAMGMT_H_ */