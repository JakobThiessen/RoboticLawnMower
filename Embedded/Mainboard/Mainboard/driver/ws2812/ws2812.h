/*
* light weight WS2812 lib include
*
* Version 2.3  - Nev 29th 2015
* Author: Tim (cpldcpu@gmail.com)
*
* Please do not change this file! All configuration is handled in "ws2812_config.h"
*
* License: GNU GPL v2+ (see License.txt)
+
*/

#ifndef LIGHT_WS2812_H_
#define LIGHT_WS2812_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "ws2812_config.h"
//#include "imath.h"
//#include "debug.h"

///////////////////////////////////////////////////////////////////////
// Define Reset time in µs.
//
// This is the time the library spends waiting after writing the data.
//
// WS2813 needs 300 µs reset time
// WS2812 and clones only need 50 µs

#define ws2812_resettime  50


///////////////////////////////////////////////////////////////////////

typedef enum {
	Black	= 0x000000,
	White	= 0xFFFFFF,
	Red		= 0xFF0000,
	Lime	= 0x00FF00,
	Blue	= 0x0000FF,
	Yellow	= 0xFFFF00,
	Cyan 	= 0x00FFFF,
	Magenta	= 0xFF00FF,
	Silver	= 0xC0C0C0,
	Gray	= 0x808080,
	Maroon	= 0x800000,
	Olive	= 0x808000,
	Green	= 0x008000,
	Purple	= 0x800080,
	Teal	= 0x008080,
	Navy	= 0x000080
} rgbColor;

/*
*  Structure of the LED array
*
* cRGB:     RGB  for WS2812S/B/C/D, SK6812, SK6812Mini, SK6812WWA, APA104, APA106
* cRGBW:    RGBW for SK6812RGBW
*/

typedef struct cRGB  { uint8_t g; uint8_t r; uint8_t b; } rgb_t;
struct cRGBW { uint8_t g; uint8_t r; uint8_t b; uint8_t w;};

inline void rgbSet(rgb_t* led, uint32_t color)
{
	led->r = (color & 0x00FF0000) >> 16;
	led->g = (color & 0x0000FF00) >> 8;
	led->b = (color & 0x000000FF);
}

// Get how much light the LED is putting out.
inline uint8_t rgbGetLuma(rgb_t* led)
{
	// Intensity WS2812B: R = 0.6 cd, G = 1.2 cd, B = 0.3 cd
	// Y = 0.286 R + 0.571 G + 0.143 B
	//        73       146        36
	uint16_t luma = (uint16_t)73 * led->r + (uint16_t)146 * led->g + (uint16_t)36 * led->b;
	return luma >> 8;
}

inline void rgbCorrectValence_ws2812_theorie(rgb_t* led)
{
	// Lw(RGB) = (0.177, 0,812, 0,011) cd / m^2 
	// Y_ws2812b = 0.619 R + 1.422 G + 0.077 B
	//               158       364        20
	led->r = ((uint16_t)led->r * 158) >> 8;
	led->g = ((uint16_t)led->g * 364) >> 8;
	led->b = ((uint16_t)led->b * 20) >> 8;
}

// Anpassung der LED-Spektralwerte auf den RGB Farbraum um die erwünschte Farbwahrnehmung zu erreichen.
// Die Primärvalenzen der LED und des RGB-Farbraumes unterscheiden sich.
// Die Helligkeit wird um 10% reduziert, um ein Überlauf zu verhindern.
inline void rgbCorrectValence_ws2812(rgb_t* led)
{
	uint8_t max = led->r;
	if (led->g > max) { max = led->g; };
	if (led->b > max) { max = led->b; };
	
	// Lw(RGB) = (0.27, 0,63, 0,10) cd / m^2 
	// Y_ws2812b = 0.944 R + 1.103 G + 0.699 B
	//               242       282       179
	//               220       256       162
	led->r = ((uint16_t)led->r * 220) >> 8;
	led->g = ((uint16_t)led->g * 256) >> 8;
	led->b = ((uint16_t)led->b * 162) >> 8;
}

// Set the brightness (0 .. 255) with gamma correction.
inline void rgbSetBrightness(rgb_t* led, uint8_t brightness)
{
	// Sensitive range due to gamma correction: 9 .. 84
	brightness += 8;
	brightness = ((uint16_t)brightness * 192) >> 8; // * 0.75
	
	// Gamma correction, gamma = 2.5;
	uint16_t sqrtx16 = sqrt16((uint16_t)brightness * 256);
	brightness = ((uint32_t)brightness * brightness * sqrtx16 + 2048) >> 12;
	
	// Lw(RGB) = (0.27, 0,63, 0,10) cd / m^2 
	// Y_ws2812b = 0.944 R + 1.103 G + 0.699 B
	//               242       282       179
	//               220       256       162
	led->r = ((uint16_t)led->r * brightness) >> 8;
	led->g = ((uint16_t)led->g * brightness) >> 8;
	led->b = ((uint16_t)led->b * brightness) >> 8;
}

typedef struct
{
	volatile uint8_t*	port; // e.g. PORTB
	uint8_t				mask;
} ws2812_t;

void ws2812_Open(ws2812_t* handle, volatile uint8_t* portAddress, uint8_t pinMask);
void ws2812_Write(ws2812_t* handle, uint8_t* data, uint16_t datlen);

/* User Interface
*
* Input:
*         ledarray:           An array of GRB data describing the LED colors
*         number_of_leds:     The number of LEDs to write
*         pinmask (optional): Bitmask describing the output bin. e.g. _BV(PB0)
*
* The functions will perform the following actions:
*         - Set the data-out pin as output
*         - Send out the LED data
*         - Wait 50µs to reset the LEDs
*/

/*
* Internal defines
*/

#define CONCAT(a, b)            a ## b
#define CONCAT_EXP(a, b)   CONCAT(a, b)

#define ws2812_PORTREG  ws2812_port

#endif /* LIGHT_WS2812_H_ */
