/*
* light weight WS2812 lib V2.0b
*
* Controls WS2811/WS2812/WS2812B RGB-LEDs
* Author: Tim (cpldcpu@gmail.com)
*
* Jan 18th, 2014  v2.0b Initial Version
* Nov 29th, 2015  v2.3  Added SK6812RGBW support
*
* License: GNU GPL v2+ (see License.txt)
*/

#include <avr/io.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "ws2812.h"

__inline__ __attribute__((always_inline)) void ws2812_sendarray_mask(uint8_t* data, uint16_t datlen, volatile uint8_t* port, uint8_t maskhi);

void ws2812_Open(ws2812_t* handle, volatile uint8_t* portAddress, uint8_t pinMask)
{
	handle->port = portAddress;
	handle->mask = pinMask;
	
	*(portAddress - 1) |= pinMask;	// Set DDR
	*portAddress &= ~pinMask;		// Set pin low
}

void ws2812_Write(ws2812_t* handle, uint8_t* data, uint16_t datlen)
{
	ws2812_sendarray_mask(data, datlen, handle->port, handle->mask);
}

/*
This routine writes an array of bytes with RGB values to the Dataout pin
using the fast 800kHz clockless WS2811/2812 protocol.
*/

// Timing in ns
#define w_zeropulse   350
#define w_onepulse    700
#define w_totalperiod 1250

// Fixed cycles used by the inner loop
#define w_fixedlow    2 /* high pulse of '0' bit */
#define w_fixedhigh   4 /* high pulse of '1' bit */
#define w_fixedtotal  8 /* minimum time slots for one period */

// Insert NOPs to match the timing, if possible
#define w_zerocycles    (((F_CPU / 1000) * w_zeropulse	 + 500000) / 1000000)
#define w_onecycles     (((F_CPU / 1000) * w_onepulse    + 500000) / 1000000)
#define w_totalcycles   (((F_CPU / 1000) * w_totalperiod + 500000) / 1000000)

// w1 - nops between rising edge and falling edge - low
#define w1 (w_zerocycles - w_fixedlow)
// w2   nops between fe low and fe high
#define w2 (w_onecycles - w_fixedhigh - w1)
// w3   nops to complete loop
//#define w3 (w_totalcycles-w_fixedtotal-w1-w2)
#if w_totalcycles > (w_fixedtotal + w1 + w2)
#define w3 (w_totalcycles - w_fixedtotal - w1 - w2)
#else
#define w3 0 /* Korrektur Maik */
#endif

#if w1 > 0
#define w1_nops w1
#else
#define w1_nops  0
#endif

// The only critical timing parameter is the minimum pulse length of the "0"
// Warn or throw error if this timing can not be met with current F_CPU settings.
#define w_lowtime ((w1_nops + w_fixedlow) * 1000000) / (F_CPU / 1000)
#if w_lowtime > 550
#error "Light_ws2812: Sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif w_lowtime > 450
#warning "Light_ws2812: The timing is critical and may only work on WS2812B, not on WS2812(S)."
#warning "Please consider a higher clockspeed, if possible"
#endif

#if w2 > 0
#define w2_nops w2
#else
#define w2_nops  0
#endif

#if w3 > 0
#define w3_nops w3
#else
#define w3_nops  0
#endif

#define w_nop1  "nop      \n\t"
#define w_nop2  "rjmp .+0 \n\t"
#define w_nop4  w_nop2 w_nop2
#define w_nop8  w_nop4 w_nop4
#define w_nop16 w_nop8 w_nop8

void ws2812_sendarray_mask(uint8_t* data, uint16_t datlen, volatile uint8_t* port, uint8_t maskhi)
{
	uint8_t curbyte, ctr, masklo;
	uint8_t sreg_prev;
	
	sreg_prev = SREG;
	cli();
	
	masklo	= *port & ~maskhi;
	maskhi |= *port;

	while (datlen--)
	{
		curbyte = *data++;
		
		asm volatile(
			"       ldi   %0,8  \n\t"
			"loop%=:            \n\t"
			"       out   %2,%3 \n\t"    //  '1' [01] '0' [01] - re
			#if (w1_nops&1)
			w_nop1
			#endif
			#if (w1_nops&2)
			w_nop2
			#endif
			#if (w1_nops&4)
			w_nop4
			#endif
			#if (w1_nops&8)
			w_nop8
			#endif
			#if (w1_nops&16)
			w_nop16
			#endif
			"       sbrs  %1,7  \n\t"    //  '1' [03] '0' [02]
			"       out   %2,%4 \n\t"    //  '1' [--] '0' [03] - fe-low
			"       lsl   %1    \n\t"    //  '1' [04] '0' [04]
			#if (w2_nops&1)
			w_nop1
			#endif
			#if (w2_nops&2)
			w_nop2
			#endif
			#if (w2_nops&4)
			w_nop4
			#endif
			#if (w2_nops&8)
			w_nop8
			#endif
			#if (w2_nops&16)
			w_nop16
			#endif
			"       out   %2,%4 \n\t"    //  '1' [+1] '0' [+1] - fe-high
			#if (w3_nops&1)
			w_nop1
			#endif
			#if (w3_nops&2)
			w_nop2
			#endif
			#if (w3_nops&4)
			w_nop4
			#endif
			#if (w3_nops&8)
			w_nop8
			#endif
			#if (w3_nops&16)
			w_nop16
			#endif

			"       dec   %0    \n\t"    //  '1' [+2] '0' [+2]
			"       brne  loop%=\n\t"    //  '1' [+3] '0' [+4]
			:		"=&d" (ctr)
			:		"r" (curbyte), "I" (_SFR_IO_ADDR(ws2812_PORTREG)), "r" (maskhi), "r" (masklo)
		);
	}
	
	SREG = sreg_prev;
}