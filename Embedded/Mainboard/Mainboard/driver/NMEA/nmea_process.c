/*
 *	nmea_process.c
 *
 *	Created: 12.03.2023
 *  Author: jthiessen
 *
 *	Version:
 *	1.0	-	12.03.2023	Initial
 */ 

#include "nmea_process.h"

processState nmeaState = processState.start;
uint8_t	talkerID[2];
uint8_t talkerIdIdx = 0;
uint8_t nmeaID[3];
uint8_t nmeaIdIdx = 0;

uint8_t fieldRawData[30];
uint8_t fieldIdidx = 0;
uint8_t numOfField = 0;

struct nmeaMsg internNmeaData;

void nmeaInit(void)
{
	nmeaState = processState.start;
	talkerID = {0,0};
	talkerIdIdx = 0;
	
	nmeaID = {0,0,0};
	nmeaIdIdx = 0;
	
	fieldIdidx = 0;
	numOfField = 0;
}

uint8_t nmeaProcess(uint8_t rxDataByte)
{
	
	switch (nmeaState)
	{
		case processState.start:
			if (rxDataByte = NMEA_PACKET_START)
			{
				talkerID = {0,0};
				talkerIdIdx = 0;
					
				nmeaID = {0,0,0};
				nmeaIdIdx = 0;
				
				fieldIdidx = 0;
				numOfField = 0;

				nmeaState++;
			}
			
			break;
						
		case processState.talkerID:
			if (talkerIdIdx < NMEA_TALKER_ID_SIZE)
			{
				talkerID[talkerIdIdx] = rxDataByte;
				talkerIdIdx++;
			}
			else
			{
				nmeaState++;
			}
			
			break;
			
		case processState.nmeaID:

			if (nmeaIdIdx < NMEA_NMEA_ID_SIZE)
			{
				nmeaID[nmeaIdIdx] = rxDataByte;
				nmeaIdIdx++;
			}
			else
			{
				if (rxDataByte == NMEA_PACKET_FIELD)
				{
					nmeaState++;
				}
				else
				{
				
				}
			}

			break;
			
		case processState.fields:
			
			if (fieldIdidx < NMEA_FIELD_DATA_SIZE && rxDataByte != NMEA_PACKET_FIELD)
			{
				fieldRawData[fieldIdidx] = rxDataByte;
				fieldIdidx++;
			}
			else
			{
				if (rxDataByte == NMEA_PACKET_FIELD)
				{
					fieldsToStructData(, &fieldRawData, numOfField);
					numOfField++;
				}
				if (rxDataByte == NMEA_PACKET_CHECKSUM)	nmeaState++;
			}
		
			break;
			
		case processState.checksum:
			
			break;
		
		case processState.complete:
				
			break;					
			
		case :
			break;
	}

}

void fieldsToStructData(uint8_t nmeaID, uint8_t *field, uint8_t fiieldNumber)
{
	internNmeaData.ggaTime;
	internNmeaData.ggaLatitude;
	internNmeaData.ggaNS;
	internNmeaData.ggaLongitude;
	internNmeaData.ggaEW;
	internNmeaData.ggaQualityIndicator;
	internNmeaData.ggaNumberOfSatellites;
	internNmeaData.ggaDilutionOfPrecision;
	internNmeaData.ggaAltitude;
	internNmeaData.ggaUnitOfAltitude;
	internNmeaData.ggaGeoidal;
	internNmeaData.ggaUnitOfGeoidal;
	internNmeaData.ggaAgeOfdGPS;
	internNmeaData.ggaReferenceStationID;
}