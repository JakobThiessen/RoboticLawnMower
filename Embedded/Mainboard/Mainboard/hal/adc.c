/*
 * adc.c
 *
 * Created: 12/10/2020 1:11:14 PM
 * Author : JThiessen
 */ 

#include "adc.h"

/* This function initializes the VREF module */
void VREF0_init(enum VREF_REFSEL_enum vref)
{
    VREF.ADC0REF = vref;  /* Internal 2.048V reference */
}

/* This function initializes the ADC module */
void ADC0_init(enum VREF_REFSEL_enum vref)
{
    ADC0.CTRLC = ADC_PRESC_DIV4_gc;         /* CLK_PER divided by 4 */
    ADC0.CTRLA = ADC_ENABLE_bm              /* ADC Enable: enabled */
                | ADC_RESSEL_12BIT_gc       /* 12-bit mode */
                | ADC_FREERUN_bm;           /* Free-run mode */
    ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;  /* Select ADC channel, Temp. */
	
	VREF0_init(vref);
}

void ADC0_selectChannel(enum ADC_MUXPOS_enum mux)
{
	ADC0.MUXPOS = mux;
}

/* This function returns the ADC conversion result */
uint16_t ADC0_read(void)
{
    /* Wait for ADC result to be ready */
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    /* Clear the interrupt flag by reading the result */
    return ADC0.RES;
}

/* This function returns the temperature value in degrees C */
int16_t temperatureConvert(uint16_t data)
{
    uint16_t sigrow_offset = SIGROW.TEMPSENSE1;
    uint16_t sigrow_slope = SIGROW.TEMPSENSE0;
    int32_t temp;
    /* Clear the interrupt flag by reading the result (ADC0.RES) */
    temp = sigrow_offset - data;
    /* Result will overflow 16-bit variable */
    temp *= sigrow_slope; 
    /* Add 4096/2 to get correct rounding on division below */
    temp += 0x0800;       
    /* Round off to nearest degree in Kelvin, by dividing with 2^12 (4096) */
    temp >>= 12;          
    /* Convert from Kelvin to Celsius (0 Kelvin - 273.15 = -273.1°C) */
    return temp - 273;      
}

/* This function starts the ADC conversions*/
void ADC0_start(void)
{
    /* Start conversion */
    ADC0.COMMAND = ADC_STCONV_bm;
}