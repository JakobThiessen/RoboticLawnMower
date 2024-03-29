/*
 * ssd1306.c
 *
 * Created: 19.06.2020 11:36:46
 *  Author: jakob
 */ 

#include "ssd1306.h"
#include "glcdfont.c"
#include <string.h>

/* Timeout counter. */
uint16_t timeout = 0;

static uint8_t ssd1306_buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8];

int8_t ssd1306_command(struct ssd1306_dev_t *dev, uint8_t c)
{
	uint8_t buf[2] = {0, 0};
	
	buf[0] = 0x00;		// Co = 0, D/C = 0
	buf[1] = c;

	int8_t rslt = dev->write(dev->id, buf, 2, true);
	
	return rslt;
}

void SSD1306_init(struct ssd1306_dev_t *dev)
{
	// Init sequence
	ssd1306_command(dev, SSD1306_DISPLAYOFF);                    // 0xAE
	ssd1306_command(dev, SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
	ssd1306_command(dev, 0x80);                                  // the suggested ratio 0x80

	ssd1306_command(dev, SSD1306_SETMULTIPLEX);                  // 0xA8
	ssd1306_command(dev, SSD1306_LCDHEIGHT - 1);

	ssd1306_command(dev, SSD1306_SETDISPLAYOFFSET);              // 0xD3
	ssd1306_command(dev, 0x0);                                   // no offset
	ssd1306_command(dev, SSD1306_SETSTARTLINE | 0x0);            // line #0
	ssd1306_command(dev, SSD1306_CHARGEPUMP);                    // 0x8D
	if (dev->vccstate == SSD1306_EXTERNALVCC)
	{ ssd1306_command(dev, 0x10); }
	else
	{ ssd1306_command(dev, 0x14); }
	ssd1306_command(dev, SSD1306_MEMORYMODE);                    // 0x20
	ssd1306_command(dev, 0x00);                                  // 0x0 act like ks0108
	ssd1306_command(dev, SSD1306_SEGREMAP | 0x1);
	ssd1306_command(dev, SSD1306_COMSCANDEC);

	#if defined SSD1306_128_32
	ssd1306_command(dev, SSD1306_SETCOMPINS);                    // 0xDA
	ssd1306_command(dev, 0x02);
	ssd1306_command(dev, SSD1306_SETCONTRAST);                   // 0x81
	ssd1306_command(dev, 0x8F);

	#elif defined SSD1306_128_64
	ssd1306_command(dev, SSD1306_SETCOMPINS);                    // 0xDA
	ssd1306_command(dev, 0x12);
	ssd1306_command(dev, SSD1306_SETCONTRAST);                   // 0x81
	if (dev->vccstate == SSD1306_EXTERNALVCC)
	{ ssd1306_command(dev, 0x9F); }
	else
	{ ssd1306_command(dev, 0xCF); }

	#elif defined SSD1306_96_16
	ssd1306_command(dev, SSD1306_SETCOMPINS);                    // 0xDA
	ssd1306_command(dev, 0x2);   //ada x12
	ssd1306_command(dev, SSD1306_SETCONTRAST);                   // 0x81
	if (dev->vccstate == SSD1306_EXTERNALVCC)
	{ ssd1306_command(dev, 0x10); }
	else
	{ ssd1306_command(dev, 0xAF); }

	#endif

	ssd1306_command(dev, SSD1306_SETPRECHARGE);                  // 0xd9
	if (dev->vccstate == SSD1306_EXTERNALVCC)
	{ ssd1306_command(dev, 0x22); }
	else
	{ ssd1306_command(dev, 0xF1); }
	ssd1306_command(dev, SSD1306_SETVCOMDETECT);                 // 0xDB
	ssd1306_command(dev, 0x40);
	ssd1306_command(dev, SSD1306_DISPLAYALLON_RESUME);           // 0xA4
	ssd1306_command(dev, SSD1306_NORMALDISPLAY);                 // 0xA6

	ssd1306_command(dev, SSD1306_DEACTIVATE_SCROLL);

	ssd1306_command(dev, SSD1306_DISPLAYON);//--turn on oled panel
	
	// set cursor to (0, 0)
	x_pos = 0;
	y_pos = 0;
	// set text size to 1
	text_size = 1;
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool color)
{
  if ((x >= SSD1306_LCDWIDTH) || (y >= SSD1306_LCDHEIGHT))
    return;
  if (color)
    ssd1306_buffer[x + (uint16_t)(y / 8) * SSD1306_LCDWIDTH] |=  (1 << (y & 7));
  else
    ssd1306_buffer[x + (uint16_t)(y / 8) * SSD1306_LCDWIDTH] &=  ~(1 << (y & 7));
}

void SSD1306_StartScrollRight(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop)
{
  ssd1306_command(dev, SSD1306_RIGHT_HORIZONTAL_SCROLL);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, start);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, stop);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, 0XFF);
  ssd1306_command(dev, SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_StartScrollLeft(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop)
{
  ssd1306_command(dev, SSD1306_LEFT_HORIZONTAL_SCROLL);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, start);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, stop);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, 0XFF);
  ssd1306_command(dev, SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_StartScrollDiagRight(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop)
{
  ssd1306_command(dev, SSD1306_SET_VERTICAL_SCROLL_AREA);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, SSD1306_LCDHEIGHT);
  ssd1306_command(dev, SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, start);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, stop);
  ssd1306_command(dev, 0X01);
  ssd1306_command(dev, SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_StartScrollDiagLeft(struct ssd1306_dev_t *dev, uint8_t start, uint8_t stop)
{
  ssd1306_command(dev, SSD1306_SET_VERTICAL_SCROLL_AREA);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, SSD1306_LCDHEIGHT);
  ssd1306_command(dev, SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, start);
  ssd1306_command(dev, 0X00);
  ssd1306_command(dev, stop);
  ssd1306_command(dev, 0X01);
  ssd1306_command(dev, SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_StopScroll(struct ssd1306_dev_t *dev)
{
  ssd1306_command(dev, SSD1306_DEACTIVATE_SCROLL);
}

void SSD1306_Dim(struct ssd1306_dev_t *dev, bool dim, uint8_t vccstate)
{
  uint8_t contrast;
  if (dim)
    contrast = 0; // Dimmed display
  else {
    if (vccstate == SSD1306_EXTERNALVCC)
      contrast = 0x9F;
    else
      contrast = 0xCF;
  }
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  ssd1306_command(dev, SSD1306_SETCONTRAST);
  ssd1306_command(dev, contrast);
}


// invert the display
void SSD1306_InvertDisplay(struct ssd1306_dev_t *dev, bool i)
{
	if (i)
	ssd1306_command(dev, SSD1306_INVERTDISPLAY_);
	else
	ssd1306_command(dev, SSD1306_NORMALDISPLAY);
}

void SSD1306_FlipHorizontalDisplay(struct ssd1306_dev_t *dev, bool i)
{
	//Set MUX ratio(A8h)
	//COM Normal / Remapped (C0h / C8h)
	if (i)
	{
		ssd1306_command(dev, SSD1306_SEGREMAP | 0x01);				// 0xA1

		ssd1306_command(dev, SSD1306_COMSCANDEC);					// C8 = Remapped
		ssd1306_command(dev, SSD1306_SETDISPLAYOFFSET);              // 0xD3 (Display Offset)
		ssd1306_command(dev, 0x0);                                   // no offset
		ssd1306_command(dev, SSD1306_SETSTARTLINE | 0x0);            // Start line = 0
	}
	else
	{
		ssd1306_command(dev, SSD1306_SEGREMAP);						// 0xA0

		ssd1306_command(dev, SSD1306_COMSCANINC);					// C0 = Normal
		ssd1306_command(dev, SSD1306_SETDISPLAYOFFSET);              // 0xD3 (Display Offset)
		ssd1306_command(dev, 0x0);                                   // no offset
		ssd1306_command(dev, SSD1306_SETSTARTLINE | 0x0);            // Start line = 0
	}
}

int8_t SSD1306_Update(struct ssd1306_dev_t *dev)
{
	ssd1306_command(dev, SSD1306_COLUMNADDR);
	ssd1306_command(dev, 0);   // Column start address (0 = reset)
	ssd1306_command(dev, SSD1306_LCDWIDTH-1); // Column end address (127 = reset)

	ssd1306_command(dev, SSD1306_PAGEADDR);
	ssd1306_command(dev, 0); // Page start address (0 = reset)
	#if SSD1306_LCDHEIGHT == 64
	ssd1306_command(dev, 7); // Page end address
	#endif
	#if SSD1306_LCDHEIGHT == 32
	ssd1306_command(dev, 3); // Page end address
	#endif
	#if SSD1306_LCDHEIGHT == 16
	ssd1306_command(dev, 1); // Page end address
	#endif

	uint8_t write_buffer[1025];
	write_buffer[0] = 0x40;
	int8_t rslt = 0;
	
	memcpy(&write_buffer[1], &ssd1306_buffer[0], 1024);
	rslt += dev->write(dev->id, write_buffer, 1024, true);

	return rslt;
}

void SSD1306_ClearDisplay(void)
{
  for (uint16_t i = 0; i < (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT / 8); i++)
    ssd1306_buffer[i] = 0;
}

void SSD1306_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color)
{
  bool steep;
  int8_t ystep;
  uint8_t dx, dy;
  int16_t err;
  steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    ssd1306_swap(x0, y0);
    ssd1306_swap(x1, y1);
  }
  if (x0 > x1) {
    ssd1306_swap(x0, x1);
    ssd1306_swap(y0, y1);
  }
  dx = x1 - x0;
  dy = abs(y1 - y0);

  err = dx / 2;
  if (y0 < y1)
    ystep = 1;
  else
    ystep = -1;

  for (; x0 <= x1; x0++) {
    if (steep) {
      if(color) SSD1306_DrawPixel(y0, x0, true);
      else      SSD1306_DrawPixel(y0, x0, false);
    }
    else {
      if(color) SSD1306_DrawPixel(x0, y0, true);
      else      SSD1306_DrawPixel(x0, y0, false);
    }
    err -= dy;
    if (err < 0) {
      y0  += ystep;
      err += dx;
    }
  }
}

void SSD1306_DrawFastHLine(uint8_t x, uint8_t y, uint8_t w, bool color)
{
   SSD1306_DrawLine(x, y, x + w - 1, y, color);
}

void SSD1306_DrawFastVLine(uint8_t x, uint8_t y, uint8_t h, bool color)
{
  SSD1306_DrawLine(x, y, x, y + h - 1, color);
}

void SSD1306_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool color)
{
  for (int16_t i = x; i < x + w; i++)
    SSD1306_DrawFastVLine(i, y, h, color);
}

void SSD1306_FillScreen(bool color)
{
  SSD1306_FillRect(0, 0, SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, color);
}

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  SSD1306_DrawPixel(x0  , y0 + r, false);
  SSD1306_DrawPixel(x0  , y0 - r, false);
  SSD1306_DrawPixel(x0 + r, y0, false);
  SSD1306_DrawPixel(x0 - r, y0, false);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    SSD1306_DrawPixel(x0 + x, y0 + y, false);
    SSD1306_DrawPixel(x0 - x, y0 + y, false);
    SSD1306_DrawPixel(x0 + x, y0 - y, false);
    SSD1306_DrawPixel(x0 - x, y0 - y, false);
    SSD1306_DrawPixel(x0 + y, y0 + x, false);
    SSD1306_DrawPixel(x0 - y, y0 + x, false);
    SSD1306_DrawPixel(x0 + y, y0 - x, false);
    SSD1306_DrawPixel(x0 - y, y0 - x, false);
  }

}

void SSD1306_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      SSD1306_DrawPixel(x0 + x, y0 + y, false);
      SSD1306_DrawPixel(x0 + y, y0 + x, false);
    }
    if (cornername & 0x2) {
      SSD1306_DrawPixel(x0 + x, y0 - y, false);
      SSD1306_DrawPixel(x0 + y, y0 - x, false);
    }
    if (cornername & 0x8) {
      SSD1306_DrawPixel(x0 - y, y0 + x, false);
      SSD1306_DrawPixel(x0 - x, y0 + y, false);
    }
    if (cornername & 0x1) {
      SSD1306_DrawPixel(x0 - y, y0 - x, false);
      SSD1306_DrawPixel(x0 - x, y0 - y, false);
    }
  }

}

void SSD1306_FillCircle(int16_t x0, int16_t y0, int16_t r, bool color)
{
  SSD1306_DrawFastVLine(x0, y0 - r, 2 * r + 1, color);
  SSD1306_FillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void SSD1306_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, bool color)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x01) {
      SSD1306_DrawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
      SSD1306_DrawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
    }
    if (cornername & 0x02) {
      SSD1306_DrawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
      SSD1306_DrawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
    }
  }

}

// Draw a rectangle
void SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  SSD1306_DrawFastHLine(x, y, w, false);
  SSD1306_DrawFastHLine(x, y + h - 1, w, false);
  SSD1306_DrawFastVLine(x, y, h, false);
  SSD1306_DrawFastVLine(x + w - 1, y, h, false);
}

// Draw a rounded rectangle
void SSD1306_DrawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r)
{
  // smarter version
  SSD1306_DrawFastHLine(x + r, y, w - 2 * r, false); // Top
  SSD1306_DrawFastHLine(x + r, y + h - 1, w - 2 * r, false); // Bottom
  SSD1306_DrawFastVLine(x, y + r, h - 2 * r, false); // Left
  SSD1306_DrawFastVLine(x + w - 1, y + r, h - 2 * r, false); // Right
  // draw four corners
  SSD1306_DrawCircleHelper(x + r, y + r, r, 1);
  SSD1306_DrawCircleHelper(x + w - r - 1, y + r, r, 2);
  SSD1306_DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4);
  SSD1306_DrawCircleHelper(x + r, y + h - r - 1, r, 8);
}

// Fill a rounded rectangle
void SSD1306_FillRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, bool color)
{
  // smarter version
  SSD1306_FillRect(x + r, y, w - 2 * r, h, color);
  // draw four corners
  SSD1306_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
  SSD1306_FillCircleHelper(x + r        , y + r, r, 2, h - 2 * r - 1, color);
}

// Draw a triangle
void SSD1306_DrawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  SSD1306_DrawLine(x0, y0, x1, y1, false);
  SSD1306_DrawLine(x1, y1, x2, y2, false);
  SSD1306_DrawLine(x2, y2, x0, y0, false);
}

// Fill a triangle
void SSD1306_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool color)
{
  int16_t a, b, y, last;
  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    ssd1306_swap(y0, y1); ssd1306_swap(x0, x1);
  }
  if (y1 > y2) {
    ssd1306_swap(y2, y1); ssd1306_swap(x2, x1);
  }
  if (y0 > y1) {
    ssd1306_swap(y0, y1); ssd1306_swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    SSD1306_DrawFastHLine(a, y0, b - a + 1, color);
    return;
  }

  int16_t
  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1;
  int32_t  sa   = 0, sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1 - 1; // Skip it

  for(y = y0; y <= last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) ssd1306_swap(a, b);
    SSD1306_DrawFastHLine(a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y <= y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) ssd1306_swap(a, b);
    SSD1306_DrawFastHLine(a, y, b - a + 1, color);
  }
}

void SSD1306_SetTextWrap(bool w)
{
  wrap = w;
}

void SSD1306_DrawChar(uint8_t x, uint8_t y, uint8_t c, uint8_t size)
{
  SSD1306_GotoXY(x, y);
  SSD1306_TextSize(size);
  SSD1306_Print(c);
}

void SSD1306_DrawText(uint8_t x, uint8_t y, char *_text, uint8_t size)
{
  SSD1306_GotoXY(x, y);
  SSD1306_TextSize(size);
  while(*_text != '\0')
    SSD1306_Print(*_text++);

}

// move cursor to position (x, y)
void SSD1306_GotoXY(uint8_t x, uint8_t y)
{
  if((x >= SSD1306_LCDWIDTH) || (y >= SSD1306_LCDHEIGHT))
    return;
  x_pos = x;
  y_pos = y;
}

// set text size
void SSD1306_TextSize(uint8_t t_size)
{
  if(t_size < 1)
    t_size = 1;
  text_size = t_size;
}

/* print single char
    \a  Set cursor position to upper left (0, 0)
    \b  Move back one position
    \n  Go to start of current line
    \r  Go to line below
*/
void SSD1306_Print(uint8_t c)
{
  bool _color;
  uint8_t i, j, line;
  
  if (c == ' ' && x_pos == 0 && wrap)
    return;
  if(c == '\a') {
    x_pos = y_pos = 0;
    return;
  }
  if( (c == '\b') && (x_pos >= text_size * 6) ) {
    x_pos -= text_size * 6;
    return;
  }
  if(c == '\r') {
    x_pos = 0;
    return;
  }
  if(c == '\n') {
    y_pos += text_size * 8;
    if((y_pos + text_size * 7) > SSD1306_LCDHEIGHT)
      y_pos = 0;
    return;
  }
/*
  if((c < ' ') || (c > '~'))
    c = '?';
  */
  for(i = 0; i < 5; i++ )
  {
//    if(c < 'S')
      //line = font_5x7[(c - ' ') * 5 + i];
	  line = font_5x7[c*5+i];
//    else
//      line = font2[(c - 'S') * 5 + i];
    
    for(j = 0; j < 7; j++, line >>= 1) {
      if(line & 0x01)
        _color = true;
      else
        _color = false;
      if(text_size == 1) SSD1306_DrawPixel(x_pos + i, y_pos + j, _color);
      else               SSD1306_FillRect(x_pos + (i * text_size), y_pos + (j * text_size), text_size, text_size, _color);
    }
  }

  SSD1306_FillRect(x_pos + (5 * text_size), y_pos, text_size, 7 * text_size, false);
  
  x_pos += text_size * 6;

  if( x_pos > (SSD1306_LCDWIDTH + text_size * 6) )
    x_pos = SSD1306_LCDWIDTH;

  if (wrap && (x_pos + (text_size * 5)) > SSD1306_LCDWIDTH)
  {
    x_pos = 0;
    y_pos += text_size * 8;
    if((y_pos + text_size * 7) > SSD1306_LCDHEIGHT)
      y_pos = 0;
  }
}

// print custom char (dimension: 7x5 pixel)
void SSD1306_PutCustomC(uint8_t *c)
{
  bool _color;
  uint8_t i, j, line;
  
  for(i = 0; i < 5; i++ ) {
    line = c[i];

    for(j = 0; j < 7; j++, line >>= 1) {
      if(line & 0x01)
        _color = true;
      else
        _color = false;
      if(text_size == 1) SSD1306_DrawPixel(x_pos + i, y_pos + j, _color);
      else               SSD1306_FillRect(x_pos + (i * text_size), y_pos + (j * text_size), text_size, text_size, _color);
    }
  }

  SSD1306_FillRect(x_pos + (5 * text_size), y_pos, text_size, 7 * text_size, false);

  x_pos += (text_size * 6);

  if( x_pos > (SSD1306_LCDWIDTH + text_size * 6) )
    x_pos = SSD1306_LCDWIDTH;

  if (wrap && (x_pos + (text_size * 5)) > SSD1306_LCDWIDTH)
  {
    x_pos = 0;
    y_pos += text_size * 8;
    if((y_pos + text_size * 7) > SSD1306_LCDHEIGHT)
      y_pos = 0;
  }
}

// Keine Ahnung was die damit wollten ;-)
uint8_t bit_test(uint16_t var, uint16_t var2);
uint8_t bit_test(uint16_t var, uint16_t var2)
{
	return var & var2;
	
}

// draw BMP
void SSD1306_DrawBMP(uint8_t x, uint8_t y, uint8_t *bitmap, uint8_t w, uint8_t h)
{
	
	uint8_t i,j, byteWidth = (w+7)/8;
	
	for (j = 0; j < h; j++)
	{
		for(i=0; i < w; i++)
		{
			if(bitmap[j * byteWidth + i / 8] & (128 >> (i & 7)))
			{
				SSD1306_DrawPixel(x+i, y+j, true);
			}
			else
			{
				SSD1306_DrawPixel(x+i, y+j, false);
			}
		}
	}
}

// draw BMP stored in ROM
void SSD1306_ROMBMP(uint8_t x, uint8_t y, uint8_t *bitmap, uint8_t w, uint8_t h)
{
  for( uint16_t i = 0; i < h/8; i++ )
  {    
    for( uint16_t j = 0; j < (uint16_t)w * 8; j++ )
    {      
      if( bit_test(bitmap[j/8 + i*w], j % 8) == 1 )
        SSD1306_DrawPixel(x + j/8, y + i*8 + (j % 8), true);
      else
        SSD1306_DrawPixel(x + j/8, y + i*8 + (j % 8), false);  
    }
  }
}