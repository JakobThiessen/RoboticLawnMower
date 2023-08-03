/*
 * commonDataMgmt.c
 *
 * Created: 01.08.2023 21:22:29
 *  Author: jakob
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <string.h>

#include "commonDataMgmt.h"


void initCommonDataMgmt()
{
	uint8_t data[sizeof(glbRoboterData)];
	memset(data, 0, sizeof(glbRoboterData));
	
	memcpy(data, &glbRoboterData, sizeof(glbRoboterData));	
}

int8_t getGlbRoboterData(uint8_t *buffer, uint8_t *size)
{
	*size = 87;
	memcpy(buffer, &glbRoboterData, *size);
	return 0;
}