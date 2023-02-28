/*
 * clock.h
 *
 * Created: 12/16/2020 3:13:27 PM
 *  Author: JThiessen
 */ 


#ifndef CLOCK_H_
#define CLOCK_H_

#include <avr/io.h>
#include <stdio.h>


void CLOCK_OSCHF_crystal_PLL_8M_init(void);
void CLOCK_OSCHF_crystal_PLL_16M_init(void);

/* Frequency select select
typedef enum CLKCTRL_FREQSEL_enum
{
	CLKCTRL_FREQSEL_1M_gc = (0x00<<2),  // 1 MHz system clock
	CLKCTRL_FREQSEL_2M_gc = (0x01<<2),  // 2 MHz system clock
	CLKCTRL_FREQSEL_3M_gc = (0x02<<2),  // 3 MHz system clock
	CLKCTRL_FREQSEL_4M_gc = (0x03<<2),  // 4 MHz system clock (default)
	CLKCTRL_FREQSEL_8M_gc = (0x05<<2),	// 8 MHz system clock
	CLKCTRL_FREQSEL_12M_gc = (0x06<<2),	// 12 MHz system clock
	CLKCTRL_FREQSEL_16M_gc = (0x07<<2), // 16 MHz system clock
	CLKCTRL_FREQSEL_20M_gc = (0x08<<2), // 20 MHz system clock
	CLKCTRL_FREQSEL_24M_gc = (0x09<<2), // 24 MHz system clock
} CLKCTRL_FREQSEL_t;
*/
void CLOCK_OSCHF_crystal_PLL_init(CLKCTRL_FREQSEL_t CLKCTRL_FREQSEL);

#endif /* CLOCK_H_ */