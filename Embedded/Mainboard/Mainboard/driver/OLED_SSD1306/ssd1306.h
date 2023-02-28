/******************************************************************************
 SSD1306 OLED driver for CCS PIC C compiler (SSD1306OLED.c)                   *
 Reference: Adafruit Industries SSD1306 OLED driver and graphics library.     *
                                                                              *
 The driver is for I2C mode only.                                             *
                                                                              *
 https://simple-circuit.com/                                                   *
                                                                              *
*******************************************************************************
*******************************************************************************
 This is a library for our Monochrome OLEDs based on SSD1306 drivers          *
                                                                              *
  Pick one up today in the adafruit shop!                                     *
  ------> http://www.adafruit.com/category/63_98                              *
                                                                              *
 Adafruit invests time and resources providing this open source code,         *
 please support Adafruit and open-source hardware by purchasing               *
 products from Adafruit!                                                      *
                                                                              *
 Written by Limor Fried/Ladyada  for Adafruit Industries.                     *
 BSD license, check license.txt for more information                          *
 All text above, and the splash screen must be included in any redistribution *
*******************************************************************************/

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>
#include <stdbool.h>

//------------------------------ Definitions ---------------------------------//

#ifndef SSD1306_I2C_ADDRESS
  #define SSD1306_I2C_ADDRESS   0x7A
#endif

#if !defined SSD1306_128_32 && !defined SSD1306_96_16
#define SSD1306_128_64
#endif
#if defined SSD1306_128_32 && defined SSD1306_96_16
  #error "Only one SSD1306 display can be specified at once"
#endif

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH            128
  #define SSD1306_LCDHEIGHT            64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH            128
  #define SSD1306_LCDHEIGHT            32
#endif
#if defined SSD1306_96_16
  #define SSD1306_LCDWIDTH             96
  #define SSD1306_LCDHEIGHT            16
#endif

#define SSD1306_SETCONTRAST          0x81
#define SSD1306_DISPLAYALLON_RESUME  0xA4
#define SSD1306_DISPLAYALLON         0xA5
#define SSD1306_NORMALDISPLAY        0xA6
#define SSD1306_INVERTDISPLAY_       0xA7
#define SSD1306_DISPLAYOFF           0xAE
#define SSD1306_DISPLAYON            0xAF
#define SSD1306_SETDISPLAYOFFSET     0xD3
#define SSD1306_SETCOMPINS           0xDA
#define SSD1306_SETVCOMDETECT        0xDB
#define SSD1306_SETDISPLAYCLOCKDIV   0xD5
#define SSD1306_SETPRECHARGE         0xD9
#define SSD1306_SETMULTIPLEX         0xA8
#define SSD1306_SETLOWCOLUMN         0x00
#define SSD1306_SETHIGHCOLUMN        0x10
#define SSD1306_SETSTARTLINE         0x40
#define SSD1306_MEMORYMODE           0x20
#define SSD1306_COLUMNADDR           0x21
#define SSD1306_PAGEADDR             0x22
#define SSD1306_COMSCANINC           0xC0
#define SSD1306_COMSCANDEC           0xC8
#define SSD1306_SEGREMAP             0xA0
#define SSD1306_CHARGEPUMP           0x8D
#define SSD1306_EXTERNALVCC          0x01
#define SSD1306_SWITCHCAPVCC         0x02

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL                      0x2F
#define SSD1306_DEACTIVATE_SCROLL                    0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A

#define ssd1306_swap(a, b)	{ int16_t t = a; a = b; b = t; }
#define abs(a)              (((a) <  0 ) ? -(a) : (a))

uint8_t x_pos, y_pos, text_size;
bool wrap;

/* type definitions */
typedef int8_t (*ssd1306_com_fptr_t)(uint8_t dev_addr, uint8_t *data, uint16_t len, bool stop);
typedef void (*ssd1306_delay_fptr_t)(uint32_t period_ms);

struct ssd1306_dev_t
{
	/*! Device Id (I2C Addr)*/
	uint8_t id;

	/*! Read function pointer */
	ssd1306_com_fptr_t read;

	/*! Write function pointer */
	ssd1306_com_fptr_t write;

	/*!  Delay function pointer */
	ssd1306_delay_fptr_t delay_ms;
	
	uint8_t vccstate;
};

int8_t ssd1306_command(struct ssd1306_dev_t *dev, uint8_t c);
void SSD1306_init(struct ssd1306_dev_t *dev);

void SSD1306_StartScrollRight(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop);
void SSD1306_StartScrollLeft(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop);
void SSD1306_StartScrollDiagRight(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop);
void SSD1306_StartScrollDiagLeft(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop);
void SSD1306_StopScroll(struct ssd1306_dev_t *dev);

void SSD1306_InvertDisplay(struct ssd1306_dev_t *dev, bool i);
void SSD1306_FlipHorizontalDisplay(struct ssd1306_dev_t *dev, bool i);

void SSD1306_Dim(struct ssd1306_dev_t *dev, bool dim, uint8_t vccstate);

int8_t SSD1306_Update(struct ssd1306_dev_t *dev);

void SSD1306_ClearDisplay(void);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool color);
void SSD1306_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color);
void SSD1306_DrawFastHLine(uint8_t x, uint8_t y, uint8_t w, bool color);
void SSD1306_DrawFastVLine(uint8_t x, uint8_t y, uint8_t h, bool color);
void SSD1306_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color);
void SSD1306_FillScreen(bool color);
void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r);
void SSD1306_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername);
void SSD1306_FillCircle(int16_t x0, int16_t y0, int16_t r, bool color);
void SSD1306_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, bool color);
void SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void SSD1306_DrawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r);
void SSD1306_FillRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, bool color);
void SSD1306_DrawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void SSD1306_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool color);
void SSD1306_DrawChar(uint8_t x, uint8_t y, uint8_t c, uint8_t size);
void SSD1306_DrawText(uint8_t x, uint8_t y, char *_text, uint8_t size);
void SSD1306_TextSize(uint8_t t_size);
void SSD1306_GotoXY(uint8_t x, uint8_t y);

void SSD1306_Print(uint8_t c);
void SSD1306_PutCustomC(uint8_t *c);
void SSD1306_SetTextWrap(bool w);

void SSD1306_DrawBMP(uint8_t x, uint8_t y, uint8_t *bitmap, uint8_t w, uint8_t h);
void SSD1306_ROMBMP(uint8_t x, uint8_t y, uint8_t *bitmap, uint8_t w, uint8_t h);

//--------------------------------------------------------------------------//

#endif /* SSD1306_H_ */
/** @}*/