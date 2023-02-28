/*
 * adc.h
 *
 * Created: 12/10/2020 1:11:14 PM
 * Author : JThiessen
 */ 

#ifndef ADC_INCLUDED
#define ADC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//#include "compiler.h"
#include <avr/io.h>

void VREF0_init(void);
void ADC0_init(void);
void ADC0_start(void);
uint16_t ADC0_read(void);
int16_t temperatureConvert(uint16_t data);


#ifdef __cplusplus
}
#endif

#endif /* PORT_INCLUDED */