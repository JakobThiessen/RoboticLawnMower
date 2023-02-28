/*
 * clock.c
 *
 * Created: 12/16/2020 3:13:17 PM
 *  Author: JThiessen
 */ 

#include "clock.h"

void CLOCK_OSCHF_crystal_PLL_8M_init(void)
{
	// Set the OSCHF frequency to 16 MHz
	ccp_write_io((uint8_t *)&CLKCTRL.OSCHFCTRLA, CLKCTRL_FREQSEL_8M_gc);

	// Set the PLL to use OSCHF as source, and select 3x multiplication factor
	ccp_write_io((uint8_t *) &CLKCTRL.PLLCTRLA, CLKCTRL_MULFAC_3x_gc);
}

void CLOCK_OSCHF_crystal_PLL_16M_init(void)
{
	// Set the OSCHF frequency to 16 MHz
	ccp_write_io((uint8_t *)&CLKCTRL.OSCHFCTRLA, CLKCTRL_FREQSEL_16M_gc);

	// Set the PLL to use OSCHF as source, and select 3x multiplication factor
	ccp_write_io((uint8_t *) &CLKCTRL.PLLCTRLA, CLKCTRL_MULFAC_3x_gc);
}

void CLOCK_OSCHF_crystal_PLL_init(CLKCTRL_FREQSEL_t CLKCTRL_FREQSEL)
{
	// Set OSCHF as main clock source
	ccp_write_io(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSCHF_gc);
	    
	// Wait for main clock oscillator changing to finish
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm)
	{
		;
	}
	    
	// Change OSCHF frequency to 24 MHz
	ccp_write_io(CLKCTRL.OSCHFCTRLA, CLKCTRL_FREQSEL);
	    
	// Set the multiplication factor for PLL to 2x
	ccp_write_io(CLKCTRL.PLLCTRLA, CLKCTRL_MULFAC_2x_gc);

}