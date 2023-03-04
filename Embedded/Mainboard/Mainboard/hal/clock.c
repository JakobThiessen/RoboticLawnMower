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
	// Internal high-frequency oscillator
	ccp_write_io((void*)&(CLKCTRL.MCLKCTRLA),CLKCTRL_CLKSEL_OSCHF_gc);
	
	//PDIV 2X; PEN disabled;
	ccp_write_io((void*)&(CLKCTRL.MCLKCTRLB),0x0);
	
   	// Change OSCHF frequency to FREQSEL
	uint8_t frqsel = CLKCTRL_FREQSEL;
   	ccp_write_io((void*)&(CLKCTRL.OSCHFCTRLA), frqsel);
	  
    //TUNE 0x0;
    ccp_write_io((void*)&(CLKCTRL.OSCHFTUNE),0x0);

    //MULFAC PLL is disabled; RUNSTDBY disabled; SOURCE OSCHF;
    ccp_write_io((void*)&(CLKCTRL.PLLCTRLA),0x0);
	
	//CFDEN disabled; CFDSRC CLKMAIN; CFDTST disabled;
	ccp_write_io((void*)&(CLKCTRL.MCLKCTRLC),0x0);
	
	// System clock stability check by polling the status register.
	while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSCHFS_bm));

}

int8_t CLKCTRL_init()
{

	// ccp_write_io((void*)&(CLKCTRL.XOSC32KCTRLA),CLKCTRL_CSUT_1K_gc /* 1k cycles */
	//		 | 0 << CLKCTRL_ENABLE_bp /* Enable: disabled */
	//		 | 0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */
	//		 | 0 << CLKCTRL_SEL_bp /* Select: disabled */
	//		 | 0 << CLKCTRL_LPMODE_bp /* Low-Power Mode: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.MCLKCTRLB),CLKCTRL_PDIV_2X_gc /* 2 */
	//		 | 0 << CLKCTRL_PEN_bp /* Prescaler enable: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.PLLCTRLA),0 << CLKCTRL_RUNSTDBY_bp /* Run Standby: disabled */
	//		 | CLKCTRL_MULFAC_DISABLE_gc /* 1 */
	//		 | 0 << CLKCTRL_SOURCE_bp /* Select Source for PLL: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.MCLKCTRLC),CLKCTRL_CFDSRC_CLKMAIN_gc /* Main Clock */
	//		 | 0 << CLKCTRL_CFDEN_bp /* Setting: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.MCLKINTCTRL),CLKCTRL_INTTYPE_INT_gc /* Regular Interrupt */
	//		 | 0 << CLKCTRL_CFD_bp /* Clock Failure Detection Interrupt Enable: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.XOSCHFCTRLA),0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */
	//		 | CLKCTRL_CSUTHF_256_gc /* 256 XOSCHF cycles */
	//		 | CLKCTRL_FRQRANGE_8M_gc /* Max 8 MHz XTAL Frequency */
	//		 | 0 << CLKCTRL_SEL_bp /* Source Select: disabled */
	//		 | 0 << CLKCTRL_ENABLE_bp /* External high-frequency Oscillator: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.OSCHFCTRLA),CLKCTRL_FREQSEL_4M_gc /* 4 */
	//		 | 0 << CLKCTRL_AUTOTUNE_bp /* Auto-Tune enable: disabled */
	//		 | 0 << CLKCTRL_RUNSTDBY_bp /* Run standby: disabled */);

	// ccp_write_io((void*)&(CLKCTRL.MCLKCTRLA),CLKCTRL_CLKSEL_OSCHF_gc /* Internal high-frequency oscillator */
	//		 | 0 << CLKCTRL_CLKOUT_bp /* System clock out: disabled */);

	return 0;
}