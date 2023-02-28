/*
 * Timer.c
 *
 * Created: 12/16/2020 2:57:53 PM
 *  Author: JThiessen
 */ 

#include "timer.h"

void TCA1_init(void)
{
	PORTC.DIRSET = PIN6_bm;							/* set pin 6 of PORT C as output */
	
	PORTMUX.TCAROUTEA = PORTMUX_TCA10_bm;			/* set waveform output on PORT C */

	TCA1.SINGLE.CTRLB = TCA_SINGLE_CMP2EN_bm		/* enable compare channel 2 */
	| TCA_SINGLE_WGMODE_SINGLESLOPE_gc;				/* single-slope PWM mode */
	
	TCA1.SINGLE.PER = PERIOD_EXAMPLE_VALUE;			/* set PWM frequency*/
	
	TCA1.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc	/* set clock source (sys_clk/1) */
	| TCA_SINGLE_ENABLE_bm;							/* start timer */

}

void setPwmDutyCycle(uint16_t dutyCycle)
{
	TCA1.SINGLE.CMP2 = dutyCycle;
}