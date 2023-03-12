/*
 *	nmea_process.h
 *
 *	Created: 12.03.2023
 *	Author: jthiessen
 *
 *	Version:
 *	1.0	- 12.03.2023	initial
 */ 

#ifndef NMEA_PROCESS_H_
#define NMEA_PROCESS_H_

// Message Codes
#define NMEA_NODATA		0	// No data. Packet not available, bad, or not decoded
#define NMEA_GPGGA		1	// Global Positioning System Fix Data
#define NMEA_GPVTG		2	// Course over ground and ground speed
#define NMEA_GPGLL		3	// Geographic position - latitude/longitude
#define NMEA_GPGSV		4	// GPS satellites in view
#define NMEA_GPGSA		5	// GPS DOP and active satellites
#define NMEA_GPRMC		6	// Recommended minimum specific GPS data
#define NMEA_UNKNOWN	0xFF// Packet received but not known

/*! Reserved characters are used by NMEA0183 for the following uses: */
#define NMEA_PACKET_START		'$'		// Start delimiter
#define NMEA_PACKET_END			"\r\n"	// end of NMEA packet <CR><LF>

#define NMEA_PACKET_CHECKSUM	'*'		// Checksum delimiter
#define NMEA_PACKET_FIELD		','		// Field delimiter
#define NMEA_PACKET_TAG_BLOCK	'\\'	// '\' TAG block delimiter
#define NMEA_PACKET_ENCAPS		'!'		// Start of encapsulation sentence delimiter

/*! The main talker ID includes */
#define NMEA_TALKER_ID			"GA"	// Galileo
#define NMEA_TALKER_ID			"GP"	// GPS
#define NMEA_TALKER_ID			"GL"	// GLONASS

/*! NMEA message mainly include the following "sentences" in the NMEA message */
#define NMEA_GGA_TOCKEN			"GGA"	// Global Positioning System Fixed Data
#define NMEA_GLL_TOCKEN			"GLL"	// Geographic Position-- Latitude and Longitude
#define NMEA_GSA_TOCKEN			"GSA"	// GNSS DOP and active satellites
#define NMEA_GSV_TOCKEN			"GSV"	// GNSS satellites in view
#define NMEA_RMC_TOCKEN			"RMC"	// Recommended minimum specific GPS data
#define NMEA_VTG_TOCKEN			"VTG"	// Course over ground and ground speed

#define NMEA_TALKER_ID_SIZE		2
#define NMEA_NMEA_ID_SIZE		3
#define NMEA_FIELD_DATA_SIZE	30		//


// 			1		 2	       3  4		     5  6  7   8	9    10  11    12 13   14    15
// 			|		 |	       |  |		     |  |  |   |    |     |  |      |  |    |     |
// $--GGA,hhmmss.ss, llll.ll,  a, yyyyy.yy,  a, x, xx, x.x, x.x,  M, x.x,   M, x.x, xxxx *hh
// $GPGGA,002153.000,3342.6618,N, 11751.3858,W, 1, 10, 1.2, 27.0, M, -34.2, M,    , 0000 *5E

// Differential reference station ID, 0000-1023
//	1) Time (UTC)
//	2) Latitude
//	3) N or S (North or South)
//	4) Longitude
//	5) E or W (East or West)
//	6) GPS Quality Indicator,
//		0 - fix not available or invalid,
//		1 - GPS SPS Mode, fix valid
//		2 - Differential GPS, SPS Mode, fix valid
//		3 - Not supported
//		4 - Not supported
//		5 - Not supported
//		6 - Dead Reckoning Mode, fix valid
//	7) Number of satellites in view, 00 - 12
//	8) Horizontal Dilution of precision
//	9) Antenna Altitude above/below mean-sea-level (geoid)
//	10) Units of antenna altitude, meters
//	11) Geoidal separation, the difference between the WGS-84 earth ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level below ellipsoid
//	12) Units of geoidal separation, meters
//	13) Age of differential GPS data, time in seconds since last SC104 type 1 or 9 update, null field when DGPS is not used
//	14) Differential reference station ID, 0000-1023
//	15) Checksum

struct nmeaMsg 
{
	uint32_t	ggaTime,
	float		ggaLatitude,
	char		ggaNS,
	float		ggaLongitude,
	char		ggaEW,
	uint8_t		ggaQualityIndicator,
	uint8_t		ggaNumberOfSatellites,
	float		ggaDilutionOfPrecision,
	float		ggaAltitude,
	char		ggaUnitOfAltitude,
	float		ggaGeoidal,
	char		ggaUnitOfGeoidal,
	uint8_t		ggaAgeOfdGPS,
	uint16_t	ggaReferenceStationID,
};

enum processState
{
	start,
	talkerID,
	nmeaID,
	fields,
	checksum,
	complete
};

struct msgGPGGA 
{
	uint8_t test;
};

struct msgGPRMC
{
	uint8_t test;
};


// functions
void nmeaInit(void);
uint8_t nmeaProcess(uint8_t rxDataByte);




#endif /* NEMA_PROCESS_H_ */