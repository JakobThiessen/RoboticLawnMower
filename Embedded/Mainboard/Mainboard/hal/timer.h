/*
 * timer.h
 *
 * Created: 12/16/2020 2:58:12 PM
 *  Author: JThiessen
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define PERIOD_EXAMPLE_VALUE			(0x0FFF)
#define TOP_VALUE						0xFFFF

void TIMER_0_init(void);
void TCA1_init(void);
void setPwmDutyCycle(uint16_t dutyCycle);

#endif /* TIMER_H_ */