#include "tft_lcd.h"
#include <string.h>
#include <cstdlib>


// Clipping macro for pushImage
#define PI_CLIP                                        \
  if (_vpOoB) return;                                  \
  x+= _xDatum;                                         \
  y+= _yDatum;                                         \
                                                       \
  if ((x >= _vpW) || (y >= _vpH)) return;              \
                                                       \
  int32_t dx = 0;                                      \
  int32_t dy = 0;                                      \
  int32_t dw = w;                                      \
  int32_t dh = h;                                      \
                                                       \
  if (x < _vpX) { dx = _vpX - x; dw -= dx; x = _vpX; } \
  if (y < _vpY) { dy = _vpY - y; dh -= dy; y = _vpY; } \
                                                       \
  if ((x + dw) > _vpW ) dw = _vpW - x;                 \
  if ((y + dh) > _vpH ) dh = _vpH - y;                 \
                                                       \
  if (dw < 1 || dh < 1) return;

static void reverse(char *begin, char *end) {
    char *is = begin;
    char *ie = end - 1;
    while (is < ie) {
        char tmp = *ie;
        *ie = *is;
        *is = tmp;
        ++is;
        --ie;
    }
}

static char *ltoa(long value, char *string, int radix) {
    if (radix < 2 || radix > 16) {
        *string = 0;
        return string;
    }

    char *out     = string;
    long quotient = value > 0 ? value : -value;

    do {
        const long tmp = quotient / radix;
        *out = "01234567890abcdef"[quotient - (tmp * radix)];
        ++out;
        quotient = tmp;
    } while (quotient);

    /* apply negative sign */
    if (value < 0)
        *out++ = '-';

    reverse(string, out);
    *out = 0;
    return string;
}

static char *itoa(int value, char *str, int radix) {
    return ltoa(value, str, radix);
}

TFT_eSPI::TFT_eSPI(int16_t _w, int16_t _h) {
#ifdef TFT_CS
    this->TFT_CS = TFT_CS;
    pinMode(TFT_CS.GPIO, TFT_CS.Pin, GPIO_Mode_Out_PP);
    pinSet(TFT_CS.GPIO, TFT_CS.Pin, Bit_SET);
#endif

#ifdef TFT_DC
    this->DC = TFT_DC;
    pinMode(DC.GPIO, DC.Pin, GPIO_Mode_Out_PP);
    pinSet(DC.GPIO, DC.Pin, Bit_SET);
#endif

#ifdef TFT_RST
    this->RST = TFT_RST;
    pinMode(RST.GPIO, RST.Pin, GPIO_Mode_Out_PP);
    pinSet(RST.GPIO, RST.Pin, Bit_SET);
#endif

#ifdef TFT_BL
    this->BL = TFT_BL;
    pinMode(BL.GPIO, BL.Pin, GPIO_Mode_Out_PP);
    pinSet(BL.GPIO, BL.Pin, Bit_SET);
#endif

    _init_width  = _width   = _w; // Set by specific xxxxx_Defines.h file or by users sketch
    _init_height = _height  = _h; // Set by specific xxxxx_Defines.h file or by users sketch

    resetViewport();

    rotation    = 0;
    cursor_y    = cursor_x  = 0;
    textfont    = 1;
    textsize    = 1;
    textcolor   = bitmap_fg = 0xFFFF; // White
    textbgcolor = bitmap_bg = 0x0000; // Black
    padX        = 0; // No padding
    isDigits    = false; // No bounding box adjustment
    textwrapX   = true; // Wrap text at end of line when using print stream
    textwrapY   = false; // Wrap text at bottom of screen when using print stream
    textdatum   = TL_DATUM; // Top Left text alignment is default
    fontsloaded = 0;

    _swapBytes = true; // swap colour bytes by default

    locked          = true; // Transaction mutex lock flag to ensure begin/endTranaction pairing
    inTransaction   = false; // Flag to prevent multiple sequential functions to keep bus access open
    lockTransaction = false; // start/endWrite lock flag to allow sketch to keep SPI bus access open

    _booted = true; // Default attributes
    _cp437  = true; // Legacy GLCD font bug fix
    _utf8   = true; // UTF8 decoding enabled

    _psram_enable = false;

    addr_row = 0xFFFF; // drawPixel command length optimiser
    addr_col = 0xFFFF; // drawPixel command length optimiser

    _xPivot = 0;
    _yPivot = 0;

    //	// Legacy support for bit GPIO masks
    //	cspinmask = 0;
    //	dcpinmask = 0;
    //	wrpinmask = 0;
    //	sclkpinmask = 0;

    // Flags for which fonts are loaded
#ifdef LOAD_GLCD
    fontsloaded = 0x0002; // Bit 1 set
#endif

#ifdef LOAD_FONT2
    fontsloaded |= 0x0004; // Bit 2 set
#endif

#ifdef LOAD_FONT4
    fontsloaded |= 0x0010; // Bit 4 set
#endif

#ifdef LOAD_FONT6
    fontsloaded |= 0x0040; // Bit 6 set
#endif

#ifdef LOAD_FONT7
    fontsloaded |= 0x0080; // Bit 7 set
#endif

#ifdef LOAD_FONT8
    fontsloaded |= 0x0100; // Bit 8 set
#endif

#ifdef LOAD_FONT8N
    fontsloaded |= 0x0200; // Bit 9 set
#endif

#ifdef SMOOTH_FONT
    fontsloaded |= 0x8000; // Bit 15 set
#endif
}

/***************************************************************************************
** Function name:           resetViewport
** Description:             Reset viewport to whle TFT screen, datum at 0,0
***************************************************************************************/
void TFT_eSPI::resetViewport(void) {
    // Reset viewport to the whole screen (or sprite) area
    _vpDatum = false;
    _vpOoB   = false;
    _vpX     = 0;
    _vpY     = 0;
    _xDatum  = 0;
    _yDatum  = 0;
    _vpW     = getWidth();
    _vpH     = getHeight();
    _xWidth  = getWidth();
    _yHeight = getHeight();
}

void TFT_eSPI::init(TFT_FuncCallback writeCmd, TFT_FuncCallback writeData) {

    writeCmd_8  = writeCmd;
    writeData_8 = writeData;

    //this->Hard_Reset();	//硬件复位
    this->BackLight(1); //打开背光

    this->DELAY(100);
    this->commandList(ST7789_Init_List); //初始化

}

/***************************************************************************************
** Function name:           init (tc is tab colour for ST7735 displays only)
** Description:             Reset, then initialise the TFT display registers
***************************************************************************************/
void TFT_eSPI::DELAY(uint16_t ms) {

}

//void TFT_eSPI::spi_init()
//{
//	
//}


void TFT_eSPI::Hard_Reset(void) {

}

void TFT_eSPI::BackLight(uint8_t bl_set) {
    //	if (bl_set != 0) {
    //		pinSet(BL.GPIO, BL.Pin, Bit_SET);
    //	}
    //	else {
    //		pinSet(BL.GPIO, BL.Pin, Bit_RESET);
    //	}
}

void TFT_eSPI::commandList(const uint8_t *addr) {
    uint8_t numCommands;
    uint8_t numArgs;
    uint8_t ms;

    numCommands = pgm_read_byte(addr++); // Number of commands to follow

    while (numCommands--)                    // For each command...
    {
        writeCmd_8(pgm_read_byte(addr++)); // Read, issue command
        numArgs = pgm_read_byte(addr++); // Number of args to follow
        ms      = numArgs & TFT_INIT_DELAY; // If hibit set, delay follows args
        numArgs &= ~TFT_INIT_DELAY; // Mask out delay bit

        while (numArgs--)                    // For each argument...
        {
            writeData_8(pgm_read_byte(addr++)); // Read, issue argument
        }

        if (ms) {
            ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
            DELAY((ms == 255 ? 500 : ms));
        }
    }
}

//void TFT_eSPI::writeCmd_8(uint8_t CMD)
//{
//	pinSet(DC.GPIO, DC.Pin, Bit_RESET); //dc_cmd
//	
//	while (((this->SPI)->SR & SPI_I2S_FLAG_TXE) == RESET) ;	//wait data send end
////	*((__IO uint8_t *)&SPI->DR) = CMD;
//	(this->SPI)->DR = CMD;
//	while (((this->SPI)->SR & (SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_BSY)) != SPI_I2S_FLAG_TXE) ;		//wait spi free
//	
//	pinSet(DC.GPIO, DC.Pin, Bit_SET); //dc_data
//}

//void TFT_eSPI::writeData_8(uint8_t Data)
//{	
//	while (((this->SPI)->SR & SPI_I2S_FLAG_TXE) == RESET) ;	//wait data send end
////	*((__IO uint8_t *)&SPI->DR) = Data;
//	(this->SPI)->DR = Data;
//	while (((this->SPI)->SR & (SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_BSY)) != SPI_I2S_FLAG_TXE) ;		//wait spi free
//}

void TFT_eSPI::writeData_16(uint16_t Data) {
    writeData_8((uint8_t) (Data >> 8));
    writeData_8((uint8_t) Data);
}

void TFT_eSPI::writeData_16S(uint16_t Data) {
    writeData_8((uint8_t) Data);
    writeData_8((uint8_t) (Data >> 8));
}

void TFT_eSPI::writeData_32(uint32_t Data) {
    writeData_16((uint16_t) (Data >> 16));
    writeData_16((uint16_t) Data);
}

void TFT_eSPI::setRotation(uint8_t r) {
    writeCmd_8(TFT_MADCTL);
    rotation = r % 4;

    switch (rotation) {
    case 0:
#ifdef CGRAM_OFFSET
        if (_init_width == 135) {
            colstart = 52;
            rowstart = 40;
        }
        else {
            colstart = 0;
            rowstart = 0;
        }
#endif
        writeData_8(TFT_MAD_COLOR_ORDER);
        break;

    case 1:
#ifdef CGRAM_OFFSET
        if (_init_width == 135) {
            colstart = 40;
            rowstart = 53;
        }
        else {
            colstart = 0;
            rowstart = 0;
        }
#endif
        writeData_8(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
        break;

    case 2:
#ifdef CGRAM_OFFSET
        if (_init_width == 135) {
            colstart = 53;
            rowstart = 40;
        }
        else {
            colstart = 0;
            rowstart = 80;
        }
#endif
        writeData_8(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;

    case 3:
#ifdef CGRAM_OFFSET
        if (_init_width == 135) {
            colstart = 40;
            rowstart = 52;
        }
        else {
            colstart = 80;
            rowstart = 0;
        }
#endif
        writeData_8(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;
    }
}

/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void TFT_eSPI::drawPixel(int32_t x, int32_t y, uint32_t color) {
    if (_vpOoB) return;

    x += _xDatum; //获取窗口坐标偏移
    y += _yDatum;

    // Range checking
    if ((x < _vpX) || (y < _vpY) || (x >= _vpW) || (y >= _vpH)) return;

    /* cs_low */

    // No need to send x if it has not changed (speeds things up)
    if (addr_col != x) {
        writeCmd_8(TFT_CASET);
        writeData_16(x);
        writeData_16(x);
        addr_col = x;
    }

    // No need to send y if it has not changed (speeds things up)
    if (addr_row != y) {
        writeCmd_8(TFT_PASET);
        writeData_16(y);
        writeData_16(y);
        addr_row = y;
    }

    //fillRect(x, y, 5, 5, color);

    writeCmd_8(TFT_RAMWR); // write to RAM

    writeData_16(color);

    /* cs_high */

}

/***************************************************************************************
** Function name:           fillRect
** Description:             draw a filled rectangle
***************************************************************************************/
void TFT_eSPI::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    if (_vpOoB) return;

    x += _xDatum;
    y += _yDatum;

    // Clipping
    if ((x >= _vpW) || (y >= _vpH)) return;

    if (x < _vpX) {
        w += x - _vpX;
        x = _vpX;
    }
    if (y < _vpY) {
        h += y - _vpY;
        y = _vpY;
    }

    if ((x + w) > _vpW) w = _vpW - x;
    if ((y + h) > _vpH) h = _vpH - y;

    if ((w < 1) || (h < 1)) return;

    //begin_tft_write();

    setWindow(x, y, x + w - 1, y + h - 1);

    pushBlock(color, w * h);

    //end_tft_write();
}


/***************************************************************************************
** Function name:           setAddrWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select is high at the end of this function
void TFT_eSPI::setAddrWindow(int32_t x0, int32_t y0, int32_t w, int32_t h) {
    setWindow(x0, y0, x0 + w - 1, y0 + h - 1);
}

/***************************************************************************************
** Function name:           setWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select stays low, call begin_tft_write first. Use setAddrWindow() from sketches
void TFT_eSPI::setWindow(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    addr_row = 0xFFFF;
    addr_col = 0xFFFF;

#ifdef CGRAM_OFFSET
    x0 += colstart;
    x1 += colstart;
    y0 += rowstart;
    y1 += rowstart;
#endif

    writeCmd_8(TFT_CASET);
    writeData_16((uint16_t) x0);
    writeData_16((uint16_t) x1);

    writeCmd_8(TFT_PASET);
    writeData_16((uint16_t) y0);
    writeData_16((uint16_t) y1);

    writeCmd_8(TFT_RAMWR);
}

/***************************************************************************************
** Function name:           setSwapBytes
** Description:             Used by 16 bit pushImage() to swap byte order in colours
***************************************************************************************/
void TFT_eSPI::setSwapBytes(bool swap) {
    _swapBytes = swap;
}

/***************************************************************************************
** Function name:           getSwapBytes
** Description:             Return the swap byte order for colours
***************************************************************************************/
bool TFT_eSPI::getSwapBytes(void) {
    return _swapBytes;
}

/***************************************************************************************
** Function name:           pushBlock - for STM32
** Description:             Write a block of pixels of the same colour
***************************************************************************************/

void TFT_eSPI::pushBlock(uint16_t color, uint32_t len) {
    while (len--) {
        writeData_16(color);
    }
}

/***************************************************************************************
** Function name:           pushPixels - for STM32
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void *data_in, uint32_t len) {

    uint16_t *data = (uint16_t *) data_in;

    if (_swapBytes)
        while (len--) {
            writeData_16(*data);
            data++;
        }
    else
        while (len--) {
            writeData_16S(*data);
            data++;
        }
}

/***************************************************************************************
** Function name:           pushPixels - for STM32 and 3 byte RGB display
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels_8bit(const void *data_in, uint32_t len) {

    uint8_t *data = (uint8_t *) data_in;

    uint16_t i = 0;
    if (_swapBytes) {
        for (; i < len; i++) {
            writeData_8(data[2 * i]);
            writeData_8(data[2 * i + 1]);
        }
    } else {
        for (; i < len; i++) {
            writeData_8(data[2 * i + 1]);
            writeData_8(data[2 * i]);
        }
    }
}

/***************************************************************************************
** Function name:           pushPixels - for STM32 and 3 byte RGB display
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels_2bit(const void *data_in, uint32_t len) {

    //uint16_t *data = (uint16_t*)data_in;

    uint16_t *data = (uint16_t *) data_in;
    if (_swapBytes) {
        while (len--) {
            uint16_t color = *data >> 8 | *data << 8;
            writeData_8((color & 0xF800) >> 8);
            writeData_8((color & 0x07E0) >> 3);
            writeData_8((color & 0x001F) << 3);
            data++;
        }
    } else {
        while (len--) {
            writeData_8((*data & 0xF800) >> 8);
            writeData_8((*data & 0x07E0) >> 3);
            writeData_8((*data & 0x001F) << 3);
            data++;
        }
    }
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit colour sprite or image onto TFT
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data) {
    PI_CLIP;

    //begin_tft_write();
    //inTransaction = true;

    setWindow(x, y, x + dw - 1, y + dh - 1);

    data += dx + dy * w;

    // Check if whole image can be pushed
    if (dw == w) pushPixels(data, dw * dh);
    else {
        // Push line segments to crop image
        while (dh--) {
            pushPixels(data, dw);
            data += w;
        }
    }

    //inTransaction = lockTransaction;
    //end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit sprite or image with 1 colour being transparent
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data, uint16_t transp) {
    PI_CLIP;

    //	begin_tft_write();
    //inTransaction = true;

    data += dx + dy * w;

    int32_t xe = x + dw - 1, ye = y + dh - 1;

    uint16_t lineBuf[dw]; // Use buffer to minimise setWindow call count

    // The little endian transp color must be byte swapped if the image is big endian
    if (!_swapBytes) transp = transp >> 8 | transp << 8;

    while (dh--) {
        int32_t len = dw;
        uint16_t *ptr = data;
        int32_t  px   = x;
        bool     move = true;
        uint16_t np   = 0;

        while (len--) {
            if (transp != *ptr) {
                if (move) {
                    move = false;
                    setWindow(px, y, xe, ye);
                }
                lineBuf[np] = *ptr;
                np++;
            } else {
                move = true;
                if (np) {
                    pushPixels((uint16_t *) lineBuf, np);
                    np = 0;
                }
            }
            px++;
            ptr++;
        }
        if (np) pushPixels((uint16_t *) lineBuf, np);

        y++;
        data += w;
    }

    //inTransaction = lockTransaction;
    //end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16 bit image
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data) {
    // Requires 32 bit aligned access, so use PROGMEM 16 bit word functions
    PI_CLIP;

    //begin_tft_write();
    //inTransaction = true;

    data += dx + dy * w;

    uint16_t buffer[dw];

    setWindow(x, y, x + dw - 1, y + dh - 1);

    // Fill and send line buffers to TFT
    for (int32_t i = 0; i < dh; i++) {
        for (int32_t j = 0; j < dw; j++) {
            buffer[j] = pgm_read_word(&data[i * w + j]);
        }
        pushPixels(buffer, dw);
    }

    //inTransaction = lockTransaction;
    //end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16 bit image with 1 colour being transparent
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, uint16_t transp) {
    // Requires 32 bit aligned access, so use PROGMEM 16 bit word functions
    PI_CLIP;

    //begin_tft_write();
    //inTransaction = true;

    data += dx + dy * w;

    int32_t xe = x + dw - 1, ye = y + dh - 1;

    uint16_t lineBuf[dw];

    // The little endian transp color must be byte swapped if the image is big endian
    if (!_swapBytes) transp = transp >> 8 | transp << 8;

    while (dh--) {
        int32_t len = dw;
        uint16_t *ptr = (uint16_t *) data;
        int32_t px   = x;
        bool    move = true;

        uint16_t np = 0;

        while (len--) {
            uint16_t color = pgm_read_word(ptr);
            if (transp != color) {
                if (move) {
                    move = false;
                    setWindow(px, y, xe, ye);
                }
                lineBuf[np] = color;
                np++;
            } else {
                move = true;
                if (np) {
                    pushPixels(lineBuf, np);
                    np = 0;
                }
            }
            px++;
            ptr++;
        }
        if (np) pushPixels(lineBuf, np);

        y++;
        data += w;
    }

    //inTransaction = lockTransaction;
    //end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 bit or 4 bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data) {
    PI_CLIP;

    //begin_tft_write();
    //inTransaction = true;

    setWindow(x, y, x + dw - 1, y + dh - 1);

    data += dx + dy * w;

    // Check if whole image can be pushed
    if (dw == w) pushPixels_8bit(data, dw * dh);
    else {
        // Push line segments to crop image
        while (dh--) {
            //pushPixels(data, dw);
            pushPixels_8bit(data, dw);
            data += 2 * w;
        }
    }
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 bit or 4 bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data) {
    // Requires 32 bit aligned access, so use PROGMEM 16 bit word functions
    PI_CLIP;

    //begin_tft_write();
    //inTransaction = true;

    data += dx + dy * w;

    uint16_t buffer[dw];

    setWindow(x, y, x + dw - 1, y + dh - 1);

    uint16_t     index = 0;
    // Fill and send line buffers to TFT
    for (int32_t i     = 0; i < dh; i++) {
        for (int32_t j = 0; j < dw; j++) {
            buffer[j] = (pgm_read_byte(&data[(i * w + j) * 2]) << 8) | (pgm_read_byte(&data[(i * w + j) * 2 + 1]));
        }
        pushPixels(buffer, dw);
    }

    //inTransaction = lockTransaction;
    //end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 bit or 4 bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_eSPI::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data, bool bpp8, uint16_t *cmap) {
    PI_CLIP;

    //begin_tft_write();
    //inTransaction = true;
    bool swap = _swapBytes;

    setWindow(x, y, x + dw - 1, y + dh - 1); // Sets CS low and sent RAMWR

    // Line buffer makes plotting faster
    uint16_t lineBuf[dw];

    if (bpp8) {
        _swapBytes = false;

        uint8_t blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table

        _lastColor = -1; // Set to illegal value

        // Used to store last shifted colour
        uint8_t msbColor = 0;
        uint8_t lsbColor = 0;

        data += dx + dy * w;
        while (dh--) {
            uint32_t len = dw;
            uint8_t *ptr     = data;
            uint8_t *linePtr = (uint8_t *) lineBuf;

            while (len--) {
                uint32_t color = *ptr++;

                // Shifts are slow so check if colour has changed first
                if (color != _lastColor) {
                    //          =====Green=====     ===============Red==============
                    msbColor   = (color & 0x1C) >> 2 | (color & 0xC0) >> 3 | (color & 0xE0);
                    //          =====Green=====    =======Blue======
                    lsbColor   = (color & 0x1C) << 3 | blue[color & 0x03];
                    _lastColor = color;
                }

                *linePtr++ = msbColor;
                *linePtr++ = lsbColor;
            }

            pushPixels(lineBuf, dw);

            data += w;
        }
        _swapBytes       = swap; // Restore old value
    } else if (cmap != nullptr) // Must be 4bpp
    {
        _swapBytes = true;

        w = (w + 1) & 0xFFFE; // if this is a sprite, w will already be even; this does no harm.
        bool splitFirst = (dx & 0x01) !=
                          0; // split first means we have to push a single px from the left of the sprite / image

        if (splitFirst) {
            data += ((dx - 1 + dy * w) >> 1);
        } else {
            data += ((dx + dy * w) >> 1);
        }

        while (dh--) {
            uint32_t len = dw;
            uint8_t *ptr = data;
            uint16_t *linePtr = lineBuf;
            uint8_t  colors; // two colors in one byte
            uint16_t index;

            if (splitFirst) {
                colors = *ptr;
                index  = (colors & 0x0F);
                *linePtr++ = cmap[index];
                len--;
                ptr++;
            }

            while (len--) {
                colors = *ptr;
                index  = ((colors & 0xF0) >> 4) & 0x0F;
                *linePtr++ = cmap[index];

                if (len--) {
                    index = colors & 0x0F;
                    *linePtr++ = cmap[index];
                } else {
                    break;  // nothing to do here
                }

                ptr++;
            }

            pushPixels(lineBuf, dw);
            data += (w >> 1);
        }
        _swapBytes      = swap; // Restore old value
    } else // Must be 1bpp
    {
        _swapBytes = false;

        uint32_t     ww = (w + 7) >> 3; // Width of source image line in bytes
        for (int32_t yp = dy; yp < dy + dh; yp++) {
            uint8_t *linePtr = (uint8_t *) lineBuf;
            for (int32_t xp = dx; xp < dx + dw; xp++) {
                uint16_t col = (data[(xp >> 3)] & (0x80 >> (xp & 0x7)));
                if (col) {
                    *linePtr++ = bitmap_fg >> 8;
                    *linePtr++ = (uint8_t) bitmap_fg;
                }
                else {
                    *linePtr++ = bitmap_bg >> 8;
                    *linePtr++ = (uint8_t) bitmap_bg;
                }
            }
            data += ww;
            pushPixels(lineBuf, dw);
        }
    }

    _swapBytes = swap; // Restore old value
    //inTransaction = lockTransaction;
    //end_tft_write();
}

/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode glyph onto the screen
***************************************************************************************/
// TODO: Rationalise with TFT_eSprite
// Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_eSPI::drawChar(uint16_t uniCode, int32_t x, int32_t y) {
    return drawChar(uniCode, x, y, textfont);
}

int16_t TFT_eSPI::drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font) {
    if (_vpOoB || !uniCode) return 0;

    if (font == 1) {
#ifdef LOAD_GLCD
#ifndef LOAD_GFXFF
        drawChar(x, y, uniCode, textcolor, textbgcolor, textsize);
        return 6 * textsize;
#endif
#else
#ifndef LOAD_GFXFF
        return 0;
#endif
#endif

#ifdef LOAD_GFXFF
        drawChar(x, y, uniCode, textcolor, textbgcolor, textsize);
        if (!gfxFont) {
            // 'Classic' built-in font
#ifdef LOAD_GLCD
            return 6 * textsize;
#else
            return 0;
#endif
        }
        else {
            if ((uniCode >= pgm_read_word(&gfxFont->first)) && (uniCode <= pgm_read_word(&gfxFont->last))) {
                uint16_t   c2    = uniCode - pgm_read_word(&gfxFont->first);
                GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c2]);
                return pgm_read_byte(&glyph->xAdvance) * textsize;
            }
            else {
                return 0;
            }
        }
#endif
    }

    if ((font > 1) && (font < 9) && ((uniCode < 32) || (uniCode > 127))) return 0;

    int32_t  width         = 0;
    int32_t  height        = 0;
    uint32_t flash_address = 0;
    uniCode -= 32;

#ifdef LOAD_FONT2
    if (font == 2) {
        flash_address = pgm_read_dword(&chrtbl_f16[uniCode]);
        width = pgm_read_byte(widtbl_f16 + uniCode);
        height = chr_hgt_f16;
    }
#ifdef LOAD_RLE
    else
#endif
#endif

#ifdef LOAD_RLE
    {
        if ((font > 2) && (font < 9)) {
            flash_address = pgm_read_dword((const void*)(pgm_read_dword(&(fontdata[font].chartbl)) + uniCode*sizeof(void *)));
            width = pgm_read_byte((uint8_t *)pgm_read_dword(&(fontdata[font].widthtbl)) + uniCode);
            height = pgm_read_byte(&fontdata[font].height);
        }
    }
#endif

    int32_t xd = x + _xDatum;
    int32_t yd = y + _yDatum;

    if ((xd + width * textsize < _vpX || xd >= _vpW) && (yd + height * textsize < _vpY || yd >= _vpH))
        return width * textsize;

    int32_t w    = width;
    int32_t pX   = 0;
    int32_t pY   = y;
    uint8_t line = 0;
    bool    clip = xd < _vpX || xd + width * textsize >= _vpW || yd < _vpY || yd + height * textsize >= _vpH;

#ifdef LOAD_FONT2 // chop out code if we do not need it
    if (font == 2) {
        w = w + 6; // Should be + 7 but we need to compensate for width increment
        w = w / 8;

        if (textcolor == textbgcolor || textsize != 1 || clip) {
            //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
            inTransaction = true;

            for (int32_t i = 0; i < height; i++) {
                if (textcolor != textbgcolor) fillRect(x, pY, width * textsize, textsize, textbgcolor);

                for (int32_t k = 0; k < w; k++) {
                    line = pgm_read_byte((uint8_t *)flash_address + w * i + k);
                    if (line) {
                        if (textsize == 1) {
                            pX = x + k * 8;
                            if (line & 0x80) drawPixel(pX, pY, textcolor);
                            if (line & 0x40) drawPixel(pX + 1, pY, textcolor);
                            if (line & 0x20) drawPixel(pX + 2, pY, textcolor);
                            if (line & 0x10) drawPixel(pX + 3, pY, textcolor);
                            if (line & 0x08) drawPixel(pX + 4, pY, textcolor);
                            if (line & 0x04) drawPixel(pX + 5, pY, textcolor);
                            if (line & 0x02) drawPixel(pX + 6, pY, textcolor);
                            if (line & 0x01) drawPixel(pX + 7, pY, textcolor);
                        }
                        else {
                            pX = x + k * 8 * textsize;
                            if (line & 0x80) fillRect(pX, pY, textsize, textsize, textcolor);
                            if (line & 0x40) fillRect(pX + textsize, pY, textsize, textsize, textcolor);
                            if (line & 0x20) fillRect(pX + 2 * textsize, pY, textsize, textsize, textcolor);
                            if (line & 0x10) fillRect(pX + 3 * textsize, pY, textsize, textsize, textcolor);
                            if (line & 0x08) fillRect(pX + 4 * textsize, pY, textsize, textsize, textcolor);
                            if (line & 0x04) fillRect(pX + 5 * textsize, pY, textsize, textsize, textcolor);
                            if (line & 0x02) fillRect(pX + 6 * textsize, pY, textsize, textsize, textcolor);
                            if (line & 0x01) fillRect(pX + 7 * textsize, pY, textsize, textsize, textcolor);
                        }
                    }
                }
                pY += textsize;
            }

            //inTransaction = lockTransaction;
            //end_tft_write();
        }
        else {
            // Faster drawing of characters and background using block write

            //begin_tft_write();

            setWindow(xd, yd, xd + width - 1, yd + height - 1);

            uint8_t mask;
            for (int32_t i = 0; i < height; i++) {
                pX = width;
                for (int32_t k = 0; k < w; k++) {
                    line = pgm_read_byte((uint8_t *)(flash_address + w * i + k));
                    mask = 0x80;
                    while (mask && pX) {
                        if (line & mask) {writeData_16(textcolor); }
                        else {writeData_16(textbgcolor); }
                        pX--;
                        mask = mask >> 1;
                    }
                }
                if (pX) {writeData_16(textbgcolor); }
            }

            //end_tft_write();
        }
    }

#ifdef LOAD_RLE
    else
#endif
#endif  //FONT2

#ifdef LOAD_RLE  //674 bytes of code
    // Font is not 2 and hence is RLE encoded
{
    //begin_tft_write();
    inTransaction = true;

    w *= height; // Now w is total number of pixels in the character
    if (textcolor == textbgcolor && !clip) {

        int32_t px = 0, py = pY; // To hold character block start and end column and row values
        int32_t pc = 0; // Pixel count
        uint8_t np = textsize * textsize; // Number of pixels in a drawn pixel

        uint8_t tnp = 0; // Temporary copy of np for while loop
        uint8_t ts = textsize - 1; // Temporary copy of textsize
        // 16 bit pixel count so maximum font size is equivalent to 180x180 pixels in area
        // w is total number of pixels to plot to fill character block
        while (pc < w) {
            line = pgm_read_byte((uint8_t *)flash_address);
            flash_address++;
            if (line & 0x80) {
                line &= 0x7F;
                line++;
                if (ts) {
                    px = xd + textsize * (pc % width); // Keep these px and py calculations outside the loop as they are slow
                    py = yd + textsize * (pc / width);
                }
                else {
                    px = xd + pc % width; // Keep these px and py calculations outside the loop as they are slow
                    py = yd + pc / width;
                }
                while (line--) {
                    // In this case the while(line--) is faster
                    pc++; // This is faster than putting pc+=line before while()?
                    setWindow(px, py, px + ts, py + ts);

                    if (ts) {
                        tnp = np;
                        while (tnp--) {writeData_16(textcolor); }
                    }
                    else {writeData_16(textcolor); }
                    px += textsize;

                    if (px >= (xd + width * textsize)) {
                        px = xd;
                        py += textsize;
                    }
                }
            }
            else {
                line++;
                pc += line;
            }
        }
    }
    else {
        // Text colour != background and textsize = 1 and character is within viewport area
        // so use faster drawing of characters and background using block write
        if (textcolor != textbgcolor && textsize == 1 && !clip)
        {
            setWindow(xd, yd, xd + width - 1, yd + height - 1);

            // Maximum font size is equivalent to 180x180 pixels in area
            while (w > 0) {
                line = pgm_read_byte((uint8_t *)flash_address++); // 8 bytes smaller when incrementing here
                if (line & 0x80) {
                    line &= 0x7F;
                    line++; w -= line;
                    pushBlock(textcolor, line);
                }
                else {
                    line++; w -= line;
                    pushBlock(textbgcolor, line);
                }
            }
        }
        else
        {
            int32_t px = 0, py = 0; // To hold character pixel coords
            int32_t tx = 0, ty = 0; // To hold character TFT pixel coords
            int32_t pc = 0; // Pixel count
            int32_t pl = 0; // Pixel line length
            uint16_t pcol = 0; // Pixel color
            bool     pf = true; // Flag for plotting
            while (pc < w) {
                line = pgm_read_byte((uint8_t *)flash_address);
                flash_address++;
                if (line & 0x80) { pcol = textcolor; line &= 0x7F; pf = true; }
                else { pcol = textbgcolor; if (textcolor == textbgcolor) pf = false; }
                line++;
                px = pc % width;
                tx = x + textsize * px;
                py = pc / width;
                ty = y + textsize * py;

                pl = 0;
                pc += line;
                while (line--) {
                    pl++;
                    if ((px + pl) >= width) {
                        if (pf) fillRect(tx, ty, pl * textsize, textsize, pcol);
                        pl = 0;
                        px = 0;
                        tx = x;
                        py++;
                        ty += textsize;
                    }
                }
                if (pl && pf) fillRect(tx, ty, pl * textsize, textsize, pcol);
            }
        }
    }
    //inTransaction = lockTransaction;
    //end_tft_write();
}
// End of RLE font rendering
#endif
    return width * textsize;    // x +
}




/***************************************************************************************
** Function name:           width
** Description:             Return the pixel width of display (per current rotation)
***************************************************************************************/
// Return the size of the display (per current rotation)
int16_t TFT_eSPI::getWidth(void) {
    if (_vpDatum) return _xWidth;
    return _width;
}

/***************************************************************************************
** Function name:           height
** Description:             Return the pixel height of display (per current rotation)
***************************************************************************************/
int16_t TFT_eSPI::getHeight(void) {
    if (_vpDatum) return _yHeight;
    return _height;
}

void TFT_eSPI::drawNumber(int32_t x, int32_t y, uint32_t number, uint8_t size) {

}

void TFT_eSPI::drawChar(int32_t x, int32_t y, uint16_t c, uint32_t TextColor, uint32_t BgColor, uint8_t size) {
    int32_t xd = x + _xDatum; //获取窗口坐标偏移
    int32_t yd = y + _yDatum;

    if (c < 32) return;

#ifdef LOAD_GLCD
#ifdef LOAD_GFXFF
    if (!gfxFont) {
#endif

    if ((xd >= _vpW) || // Clip right
        (yd >= _vpH) || // Clip bottom
        ((xd + 6 * size - 1) < _vpX) || // Clip left
        ((yd + 8 * size - 1) < _vpY))   // Clip top
        return;

    bool fillbg = (BgColor != TextColor);
    bool clip   = xd < _vpX || xd + 6 * textsize >= _vpW || yd < _vpY || yd + 8 * textsize >= _vpH;

    if ((size == 1) && fillbg && !clip) {
        //字符显示不全
        uint8_t column[6];
        uint8_t mask = 0x01;
        // begin_tft_write();

        setWindow(xd, yd, xd + 5, yd + 8);

        for (int8_t i = 0; i < 5; i++) column[i] = pgm_read_byte(font + (c * 5) + i);
        column[5]                                = 0;

        for (int8_t j = 0; j < 8; j++) {
            for (int8_t k = 0; k < 5; k++) {
                if (column[k] & mask) { writeData_16(TextColor); }
                else { writeData_16(BgColor); }
            }
            mask <<= 1;
            writeData_16(BgColor);
        }

        //end_tft_write();
    } else {
        for (int8_t i = 0; i < 6; i++) {
            uint8_t line;
            if (i == 5) { line = 0x00; }
            else { line = pgm_read_byte(font + (c * 5) + i); }

            if (size == 1 && !fillbg) {
                // default size
                for (int8_t j = 0; j < 8; j++) {
                    if (line & 0x1) drawPixel(x + i, y + j, TextColor);
                    line >>= 1;
                }
            } else {
                // big size or clipped
                for (int8_t j = 0; j < 8; j++) {
                    if (line & 0x1) { fillRect(x + (i * size), y + (j * size), size, size, TextColor); }
                    else if (fillbg) { fillRect(x + i * size, y + j * size, size, size, BgColor); }
                    line >>= 1;
                }
            }
        }
        //end_tft_write(); // Does nothing if Sprite class uses this function
    }

#ifdef LOAD_GFXFF
    }
    else {
#endif    /*LOAD_GFXFF*/
#endif /*LOAD_GLCD*/

#ifdef LOAD_GFXFF
    // Filter out bad characters not present in font
    if ((c >= pgm_read_word(&gfxFont->first)) && (c <= pgm_read_word(&gfxFont->last))) {

        c -= pgm_read_word(&gfxFont->first);
        GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&gfxFont->bitmap);

        uint32_t bo = pgm_read_word(&glyph->bitmapOffset); //获取光相对距离和像素宽度
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);

        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;

        if (size > 1) {
            xo16 = xo;
            yo16 = yo;
        }
        // GFXFF rendering speed up
        uint16_t hpc = 0; // Horizontal foreground pixel count
        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (bit == 0) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                    bit  = 0x80;
                }
                if (bits & bit) hpc++;
                else {
                    if (hpc) {
                        if (size == 1) drawFastHLine(x + xo + xx - hpc, y + yo + yy, hpc, TextColor);
                        else fillRect(x + (xo16 + xx - hpc)*size, y + (yo16 + yy)*size, size*hpc, size, TextColor);
                        hpc = 0;
                    }
                }
                bit >>= 1;
            }
            // Draw pixels for this line as we are about to increment yy
            if (hpc) {
                if (size == 1) drawFastHLine(x + xo + xx - hpc, y + yo + yy, hpc, TextColor);
                else fillRect(x + (xo16 + xx - hpc)*size, y + (yo16 + yy)*size, size*hpc, size, TextColor);
                hpc = 0;
            }
        }

        //end_tft_write(); // Does nothing if Sprite class uses this function

    }
#endif

#ifdef LOAD_GLCD
#ifdef LOAD_GFXFF
    } // End classic vs custom font
#endif
#endif
}

/***************************************************************************************
** Function name:           drawLine
** Description:             draw a line between 2 arbitrary points
***************************************************************************************/
// Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
// an efficient FastH/V Line draw routine for line segments of 2 pixels or more
void TFT_eSPI::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {

}

/***************************************************************************************
** Function name:           drawFastVLine
** Description:             draw a vertical line
***************************************************************************************/
void TFT_eSPI::drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color) {
    if (_vpOoB) return;

    x += _xDatum;
    y += _yDatum;

    // Clipping
    if ((x < _vpX) || (x >= _vpW) || (y >= _vpH)) return;

    if (y < _vpY) {
        h += y - _vpY;
        y = _vpY;
    }

    if ((y + h) > _vpH) h = _vpH - y;

    if (h < 1) return;

    //begin_tft_write();

    setWindow(x, y, x, y + h - 1);

    pushBlock(color, h);

    //end_tft_write();
}

/***************************************************************************************
** Function name:           drawFastHLine
** Description:             draw a horizontal line
***************************************************************************************/
void TFT_eSPI::drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color) {
    if (_vpOoB) return;

    x += _xDatum;
    y += _yDatum;

    // Clipping
    if ((y < _vpY) || (x >= _vpW) || (y >= _vpH)) return;

    if (x < _vpX) {
        w += x - _vpX;
        x = _vpX;
    }

    if ((x + w) > _vpW) w = _vpW - x;

    if (w < 1) return;

    //begin_tft_write();

    setWindow(x, y, x + w - 1, y);

    pushBlock(color, w);

    //end_tft_write();
}

/***************************************************************************************
** Function name:           fillScreen
** Description:             Clear the screen to defined colour
***************************************************************************************/
void TFT_eSPI::fillScreen(uint32_t color) {
    fillRect(0, 0, _width, _height, color);
}

/***************************************************************************************
** Function name:           invertDisplay
** Description:             invert the display colours i = 1 invert, i = 0 normal
***************************************************************************************/
void TFT_eSPI::invertDisplay(bool i) {
    //begin_tft_write();
    // Send the command twice as otherwise it does not always work!
    writeCmd_8(i ? TFT_INVON : TFT_INVOFF);
    writeCmd_8(i ? TFT_INVON : TFT_INVOFF);
    //end_tft_write();
}

void TFT_eSPI::print(int32_t x, int32_t y, const char *string) {
    print(x, y, string, textcolor, textbgcolor, textsize);
}

void TFT_eSPI::print(int32_t x, int32_t y, const char *string, uint32_t TextColor, uint32_t BgColor, uint8_t size) {
    uint16_t str_len = strlen(string);

    x += _xDatum;
    y += _yDatum;

    while (str_len--) {
        if ((uint8_t) *string >= 0xC0) {

        } else {
            drawChar(x, y, *string, TextColor, BgColor, size);

            x += 6 * size;
            string++;
        }
    }

}

/***************************************************************************************
** Function name:           drawNumber
** Description:             draw a long integer
***************************************************************************************/
int16_t TFT_eSPI::drawNumber(long long_num, int32_t poX, int32_t poY) {
    isDigits = true; // Eliminate jiggle in monospaced fonts
    char str[12];
    itoa(long_num, str, 10);
    return drawString(str, poX, poY, textfont);
}

int16_t TFT_eSPI::drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t font) {
    isDigits = true; // Eliminate jiggle in monospaced fonts
    char str[12];
    itoa(long_num, str, 10);
    return drawString(str, poX, poY, font);
}


/***************************************************************************************
** Function name:           drawFloat
** Descriptions:            drawFloat, prints 7 non zero digits maximum
***************************************************************************************/
// Assemble and print a string, this permits alignment relative to a datum
// looks complicated but much more compact and actually faster than using print class
int16_t TFT_eSPI::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY) {
    return drawFloat(floatNumber, dp, poX, poY, textfont);
}

int16_t TFT_eSPI::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t font) {
    isDigits = true;
    char    str[14]; // Array to contain decimal string
    uint8_t ptr      = 0; // Initialise pointer for array
    int8_t  digits   = 1; // Count the digits to avoid array overflow
    float   rounding = 0.5; // Round up down delta

    if (dp > 7) dp = 7; // Limit the size of decimal portion

    // Adjust the rounding value
    for (uint8_t i = 0; i < dp; ++i) rounding /= 10.0;

    if (floatNumber < -rounding) {
        // add sign, avoid adding - sign to 0.0!
        str[ptr++] = '-'; // Negative number
        str[ptr]   = 0; // Put a null in the array as a precaution
        digits      = 0; // Set digits to 0 to compensate so pointer value can be used later
        floatNumber = -floatNumber; // Make positive
    }

    floatNumber += rounding; // Round up or down

    // For error put ... in string and return (all TFT_eSPI library fonts contain . character)
    if (floatNumber >= 2147483647) {
        strcpy(str, "...");
        return drawString(str, poX, poY, font);
    }
    // No chance of overflow from here on

    // Get integer part
    uint32_t temp = (uint32_t) floatNumber;

    // Put integer part into array
    itoa(temp, str + ptr, 10);

    // Find out where the null is to get the digit count loaded
    while ((uint8_t) str[ptr] != 0) ptr++; // Move the pointer along
    digits += ptr; // Count the digits

    str[ptr++]   = '.'; // Add decimal point
    str[ptr]     = '0'; // Add a dummy zero
    str[ptr + 1] = 0; // Add a null but don't increment pointer so it can be overwritten

    // Get the decimal portion
    floatNumber = floatNumber - temp;

    // Get decimal digits one by one and put in array
    // Limit digit count so we don't get a false sense of resolution
    uint8_t i = 0;
    while ((i < dp) && (digits < 9)) {
        // while (i < dp) for no limit but array size must be increased
        i++;
        floatNumber *= 10; // for the next decimal
        temp = floatNumber; // get the decimal
        itoa(temp, str + ptr, 10);
        ptr++;
        digits++; // Increment pointer and digits count
        floatNumber -= temp; // Remove that digit
    }

    // Finally we can plot the string and return pixel length
    return drawString(str, poX, poY, font);
}

/***************************************************************************************
** Function name:           drawRightString (deprecated, use setTextDatum())
** Descriptions:            draw string right justified to dX
***************************************************************************************/
int16_t TFT_eSPI::drawString(const char *string, int32_t poX, int32_t poY) {
    return drawString(string, poX, poY, textfont);
}

int16_t TFT_eSPI::drawString(const char *string, int32_t poX, int32_t poY, uint8_t font) {
    int16_t  sumX    = 0;
    uint8_t  padding = 1, baseline = 0;
    uint16_t cwidth  = textWidth(string, font); // Find the pixel width of the string in the font
    uint16_t cheight = 8 * textsize;

#ifdef LOAD_GFXFF
#ifdef SMOOTH_FONT
    bool freeFont = (font == 1 && gfxFont && !fontLoaded);
#else
    bool freeFont = (font == 1 && gfxFont);
#endif

    if (freeFont) {
        cheight = glyph_ab * textsize;
        poY += cheight; // Adjust for baseline datum of free fonts
        baseline = cheight;
        padding = 101; // Different padding method used for Free Fonts

        // We need to make an adjustment for the bottom of the string (eg 'y' character)
        if ((textdatum == BL_DATUM) || (textdatum == BC_DATUM) || (textdatum == BR_DATUM)) {
            cheight += glyph_bb * textsize;
        }
    }
#endif


    // If it is not font 1 (GLCD or free font) get the baseline and pixel height of the font
#ifdef SMOOTH_FONT
    if (fontLoaded) {
        baseline = gFont.maxAscent;
        cheight  = fontHeight();
    }
    else
#endif
    if (font != 1) {
        baseline = pgm_read_byte(&fontdata[font].baseline) * textsize;
        cheight  = fontHeight(font);
    }

    if (textdatum || padX) {

        switch (textdatum) {
        case TC_DATUM:
            poX -= cwidth / 2;
            padding += 1;
            break;
        case TR_DATUM:
            poX -= cwidth;
            padding += 2;
            break;
        case ML_DATUM:
            poY -= cheight / 2;
            //padding += 0;
            break;
        case MC_DATUM:
            poX -= cwidth / 2;
            poY -= cheight / 2;
            padding += 1;
            break;
        case MR_DATUM:
            poX -= cwidth;
            poY -= cheight / 2;
            padding += 2;
            break;
        case BL_DATUM:
            poY -= cheight;
            //padding += 0;
            break;
        case BC_DATUM:
            poX -= cwidth / 2;
            poY -= cheight;
            padding += 1;
            break;
        case BR_DATUM:
            poX -= cwidth;
            poY -= cheight;
            padding += 2;
            break;
        case L_BASELINE:
            poY -= baseline;
            //padding += 0;
            break;
        case C_BASELINE:
            poX -= cwidth / 2;
            poY -= baseline;
            padding += 1;
            break;
        case R_BASELINE:
            poX -= cwidth;
            poY -= baseline;
            padding += 2;
            break;
        }
    }

    int8_t xo = 0;
#ifdef LOAD_GFXFF
    if (freeFont && (textcolor != textbgcolor)) {
        cheight = (glyph_ab + glyph_bb) * textsize;
        // Get the offset for the first character only to allow for negative offsets
        uint16_t c2 = 0;
        uint16_t len = strlen(string);
        uint16_t n = 0;

        while (n < len && c2 == 0) c2 = decodeUTF8((uint8_t*)string, &n, len - n);

        if ((c2 >= pgm_read_word(&gfxFont->first)) && (c2 <= pgm_read_word(&gfxFont->last))) {
            c2 -= pgm_read_word(&gfxFont->first);
            GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c2]);
            xo = pgm_read_byte(&glyph->xOffset) * textsize;
            // Adjust for negative xOffset
            if (xo > 0) xo = 0;
            else cwidth -= xo;
            // Add 1 pixel of padding all round
            //cheight +=2;
            //fillRect(poX+xo-1, poY - 1 - glyph_ab * textsize, cwidth+2, cheight, textbgcolor);
            fillRect(poX + xo, poY - glyph_ab * textsize, cwidth, cheight, textbgcolor);
        }
        padding -= 100;
    }
#endif

    uint16_t len = strlen(string);
    uint16_t n   = 0;

#ifdef SMOOTH_FONT
    if (fontLoaded) {
        if (textcolor != textbgcolor) fillRect(poX, poY, cwidth, cheight, textbgcolor);
        /*
            // The above only works for a single text line, not if the text is going to wrap...
            // So need to use code like this in a while loop to fix it:
            if (textwrapX && (cursor_x + width * textsize > width())) {
              cursor_y += height;
              cursor_x = 0;
          }
          if (textwrapY && (cursor_y >= (int32_t)height())) cursor_y = 0;
          cursor_x += drawChar(uniCode, cursor_x, cursor_y, textfont);
          */
        setCursor(poX, poY);

        while (n < len) {
            uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
            drawGlyph(uniCode);
        }
        sumX += cwidth;
        //fontFile.close();
    }
    else
#endif
    {
        while (n < len) {
            uint16_t uniCode = decodeUTF8((uint8_t *) string, &n, len - n);
            sumX += drawChar(uniCode, poX + sumX, poY, font);
        }
    }

    //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // Switch on debugging for the padding areas
    //#define PADDING_DEBUG

#ifndef PADDING_DEBUG
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    if ((padX > cwidth) && (textcolor != textbgcolor)) {
        int16_t padXc = poX + cwidth + xo;
#ifdef LOAD_GFXFF
        if (freeFont) {
            poX += xo; // Adjust for negative offset start character
            poY -= glyph_ab * textsize;
            sumX += poX;
        }
#endif
        switch (padding) {
        case 1:
            fillRect(padXc, poY, padX - cwidth, cheight, textbgcolor);
            break;
        case 2:
            fillRect(padXc, poY, (padX - cwidth) >> 1, cheight, textbgcolor);
            padXc = poX - ((padX - cwidth) >> 1);
            fillRect(padXc, poY, (padX - cwidth) >> 1, cheight, textbgcolor);
            break;
        case 3:
            if (padXc > padX) padXc = padX;
            fillRect(poX + cwidth - padXc, poY, padXc - cwidth, cheight, textbgcolor);
            break;
        }
    }

#else

    //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // This is debug code to show text (green box) and blanked (white box) areas
    // It shows that the padding areas are being correctly sized and positioned

    if ((padX > sumX) && (textcolor != textbgcolor)) {
        int16_t padXc = poX + sumX; // Maximum left side padding
#ifdef LOAD_GFXFF
        if ((font == 1) && (gfxFont)) poY -= glyph_ab;
#endif
        drawRect(poX, poY, sumX, cheight, TFT_GREEN);
        switch (padding) {
        case 1:
            drawRect(padXc, poY, padX - sumX, cheight, TFT_WHITE);
            break;
        case 2:
            drawRect(padXc, poY, (padX - sumX) >> 1, cheight, TFT_WHITE);
            padXc = (padX - sumX) >> 1;
            drawRect(poX - padXc, poY, (padX - sumX) >> 1, cheight, TFT_WHITE);
            break;
        case 3:
            if (padXc > padX) padXc = padX;
            drawRect(poX + sumX - padXc, poY, padXc - sumX, cheight, TFT_WHITE);
            break;
        }
    }
#endif
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    return sumX;
}

/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t TFT_eSPI::textWidth(const char *string) {
    return textWidth(string, textfont);
}

int16_t TFT_eSPI::textWidth(const char *string, uint8_t font) {
    int32_t  str_width = 0;
    uint16_t uniCode   = 0;

#ifdef SMOOTH_FONT

#endif
    if (font > 1 && font < 9) {
        char *widthtable = (char *) pgm_read_dword(&(fontdata[font].widthtbl)) - 32; //subtract the 32 outside the loop

        while (*string) {
            uniCode = *(string++);
            if (uniCode > 31 && uniCode < 128)
                str_width += pgm_read_byte(widthtable + uniCode); // Normally we need to subtract 32 from uniCode
            else str_width += pgm_read_byte(widthtable + 32); // Set illegal character = space width
        }
    } else {
#ifdef LOAD_GFXFF
        if (gfxFont) {
            // New font
            while (*string) {
                uniCode = decodeUTF8(*string++);
                if ((uniCode >= pgm_read_word(&gfxFont->first)) && (uniCode <= pgm_read_word(&gfxFont->last))) {
                    uniCode -= pgm_read_word(&gfxFont->first);
                    GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[uniCode]);
                    // If this is not the  last character or is a digit then use xAdvance
                    if (*string  || isDigits) str_width += pgm_read_byte(&glyph->xAdvance);
                    // Else use the offset plus width since this can be bigger than xAdvance
                    else str_width += ((int8_t)pgm_read_byte(&glyph->xOffset) + pgm_read_byte(&glyph->width));
                }
            }
        }
        else
#endif
        {
#ifdef LOAD_GLCD
            while (*string++) str_width += 6;
#endif
        }
    }
    isDigits = false;
    return str_width * textsize;
}



/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

#ifdef LOAD_GFXFF

void TFT_eSPI::setFreeFont(const GFXfont *f /* = nullptr */)
{
    if (f == nullptr) {
        // Fix issue #400 (ESP32 crash)
        setTextFont(1); // Use GLCD font
        return;
    }

    textfont = 1;
    gfxFont = (GFXfont *)f;

    glyph_ab = 0;
    glyph_bb = 0;
    uint16_t numChars = pgm_read_word(&gfxFont->last) - pgm_read_word(&gfxFont->first);

    // Find the biggest above and below baseline offsets
    for (uint8_t c = 0; c < numChars; c++) {
        GFXglyph *glyph1  = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c]);
        int8_t ab = -pgm_read_byte(&glyph1->yOffset);
        if (ab > glyph_ab) glyph_ab = ab;
        int8_t bb = pgm_read_byte(&glyph1->height) - ab;
        if (bb > glyph_bb) glyph_bb = bb;
    }
}

/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_eSPI::setTextFont(uint8_t f)
{
    textfont = (f > 0) ? f : 1; // Don't allow font 0
    gfxFont = NULL;
}
#else
/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

// Alternative to setTextFont() so we don't need two different named functions
void TFT_eSPI::setFreeFont(uint8_t font) {
    setTextFont(font);
}

/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_eSPI::setTextFont(uint8_t f) {
    textfont = (f > 0) ? f : 1; // Don't allow font 0
}

#endif

/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground colour (background is transparent)
***************************************************************************************/
void TFT_eSPI::setTextColor(uint16_t c) {
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground and background colour
***************************************************************************************/
void TFT_eSPI::setTextColor(uint16_t c, uint16_t b) {
    textcolor   = c;
    textbgcolor = b;
}

/***************************************************************************************
** Function name:           setTextSize
** Description:             Set the text size multiplier
***************************************************************************************/
void TFT_eSPI::setTextSize(uint8_t s) {
    if (s > 7) s = 7; // Limit the maximum size multiplier so byte variables can be used for rendering
    textsize = (s > 0) ? s : 1; // Don't allow font size 0
}

/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void TFT_eSPI::setTextDatum(uint8_t d) {
    textdatum = d;
}

/***************************************************************************************
** Function name:           getTextDatum
** Description:             Return the text datum value (as used by setTextDatum())
***************************************************************************************/
uint8_t TFT_eSPI::getTextDatum(void) {
    return textdatum;
}

/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Line buffer UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t TFT_eSPI::decodeUTF8(uint8_t *buf, uint16_t *index, uint16_t remaining) {
    uint16_t c = buf[(*index)++];
    //Serial.print("Byte from string = 0x"); Serial.println(c, HEX);

    if (!_utf8) return c;

    // 7 bit Unicode
    if ((c & 0x80) == 0x00) return c;

    // 11 bit Unicode
    if (((c & 0xE0) == 0xC0) && (remaining > 1))
        return ((c & 0x1F) << 6) | (buf[(*index)++] & 0x3F);

    // 16 bit Unicode
    if (((c & 0xF0) == 0xE0) && (remaining > 2)) {
        c = ((c & 0x0F) << 12) | ((buf[(*index)++] & 0x3F) << 6);
        return c | ((buf[(*index)++] & 0x3F));
    }

    // 21 bit Unicode not supported so fall-back to extended ASCII
    // if ((c & 0xF8) == 0xF0) return c;

    return c; // fall-back to extended ASCII
}

/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Serial UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t TFT_eSPI::decodeUTF8(uint8_t c) {
    if (!_utf8) return c;

    // 7 bit Unicode Code Point
    if ((c & 0x80) == 0x00) {
        decoderState = 0;
        return c;
    }

    if (decoderState == 0) {
        // 11 bit Unicode Code Point
        if ((c & 0xE0) == 0xC0) {
            decoderBuffer = ((c & 0x1F) << 6);
            decoderState  = 1;
            return 0;
        }
        // 16 bit Unicode Code Point
        if ((c & 0xF0) == 0xE0) {
            decoderBuffer = ((c & 0x0F) << 12);
            decoderState  = 2;
            return 0;
        }
        // 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
        // if ((c & 0xF8) == 0xF0) return c;
    } else {
        if (decoderState == 2) {
            decoderBuffer |= ((c & 0x3F) << 6);
            decoderState--;
            return 0;
        } else {
            decoderBuffer |= (c & 0x3F);
            decoderState = 0;
            return decoderBuffer;
        }
    }

    decoderState = 0;

    return c; // fall-back to extended ASCII
}

/***************************************************************************************
** Function name:           fontHeight
** Description:             return the height of a font (yAdvance for free fonts)
***************************************************************************************/
int16_t TFT_eSPI::fontHeight(int16_t font) {
#ifdef SMOOTH_FONT
    if (fontLoaded) return gFont.yAdvance;
#endif

#ifdef LOAD_GFXFF
    if (font == 1) {
        if (gfxFont) {
            // New font
            return pgm_read_byte(&gfxFont->yAdvance) * textsize;
        }
    }
#endif
    return pgm_read_byte(&fontdata[font].height) * textsize;
}

int16_t TFT_eSPI::fontHeight(void) {
    return fontHeight(textfont);
}

