#ifndef __TFT_LCD_H
#define __TFT_LCD_H

//#ifdef __cplusplus
//extern "C"
//{
//#endif

/***************************************************************************************
**                         Section 1: Load required header files
***************************************************************************************/
#include <stdint.h>
#include "user_setup.h"

typedef void(*TFT_FuncCallback)(uint8_t data);
/***************************************************************************************
**                         Section 2: Load library and processor specific header files
***************************************************************************************/
// Include header file that defines the fonts loaded, the TFT drivers
// available and the pins to be used, etc, etc


// Handle FLASH based storage e.g.PROGMEM
#if defined(ARDUINO_ARCH_RP2040)
#undef pgm_read_byte
#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))
#undef pgm_read_word
#define pgm_read_word(addr) ({ \
            typeof(addr) _addr = (addr); \
            *(const unsigned short *)(_addr); \
         })
#undef pgm_read_dword
#define pgm_read_dword(addr) ({ \
                   typeof(addr) _addr = (addr); \
                   *(const unsigned long *)(_addr); \
              })
#elif defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#define PROGMEM
#endif

/***************************************************************************************
**                         Section 4: Setup fonts
***************************************************************************************/
// Use GLCD font in error case where user requests a smooth font file
// that does not exist (this is a temporary fix to stop ESP32 reboot)
#ifdef SMOOTH_FONT
#ifndef LOAD_GLCD
#define LOAD_GLCD
#endif
#endif

// Only load the fonts defined in User_Setup.h (to save space)
// Set flag so RLE rendering code is optionally compiled
#ifdef LOAD_GLCD

#include "Fonts/glcdfont.c"

#endif

#ifdef LOAD_FONT2
#include <Fonts/Font16.h>
#endif

#ifdef LOAD_FONT4
#include <Fonts/Font32rle.h>
#define LOAD_RLE
#endif

#ifdef LOAD_FONT6
#include <Fonts/Font64rle.h>
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#endif

#ifdef LOAD_FONT7
#include <Fonts/Font7srle.h>
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#endif

#ifdef LOAD_FONT8
#include <Fonts/Font72rle.h>
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#elif defined LOAD_FONT8N // Optional narrower version
#define LOAD_FONT8
#include <Fonts/Font72x53rle.h>
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#endif

#ifdef LOAD_GFXFF
// We can include all the free fonts and they will only be built into
// the sketch if they are used
#include <Fonts/GFXFF/gfxfont.h>
                // Call up any user custom fonts
                //#include <User_Setups/User_Custom_Fonts.h>
                  // New custom font file #includes
#include <Fonts/Custom/Orbitron_Light_24.h> // CF_OL24
#include <Fonts/Custom/Orbitron_Light_32.h> // CF_OL32
#include <Fonts/Custom/Roboto_Thin_24.h>    // CF_RT24
#include <Fonts/Custom/Satisfy_24.h>        // CF_S24
#include <Fonts/Custom/Yellowtail_32.h>     // CF_Y32

#endif // #ifdef LOAD_GFXFF

// Create a null default font in case some fonts not used (to prevent crash)
const uint8_t         widtbl_null[1] = {0};
PROGMEM const uint8_t chr_null[1]    = {0};
PROGMEM const uint8_t *const chrtbl_null[1] = {chr_null};

// This is a structure to conveniently hold information on the default fonts
// Stores pointer to font character image address table, width table and height
typedef struct {
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    uint8_t       height;
    uint8_t       baseline;
}                     fontinfo;

// Now fill the structure
const PROGMEM fontinfo fontdata[] = {
#ifdef LOAD_GLCD
        {(const uint8_t *) font, widtbl_null, 0, 0},
#else
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},
#endif
        // GLCD font (Font 1) does not have all parameters
        {(const uint8_t *) chrtbl_null, widtbl_null, 8, 7},

#ifdef LOAD_FONT2
        { (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16 },
#else
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},
#endif

        // Font 3 current unused
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},

#ifdef LOAD_FONT4
        { (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32 },
#else
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},
#endif

        // Font 5 current unused
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},

#ifdef LOAD_FONT6
        { (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64 },
#else
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},
#endif

#ifdef LOAD_FONT7
        { (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s },
#else
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0},
#endif

#ifdef LOAD_FONT8
        { (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72 }
#else
        {(const uint8_t *) chrtbl_null, widtbl_null, 0, 0}
#endif
};

/***************************************************************************************
**                         Section 5: Font datum enumeration
***************************************************************************************/
//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right
#define L_BASELINE  9 // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

/***************************************************************************************
**                         Section 6: Colour enumeration
***************************************************************************************/
// Default color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */    //
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */
#define TFT_LIGHTPINK    0xFC9F
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */
#define RGB565(r, g, b)       (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
#define RGB555(r, g, b)          (((r & 0xF8) << 7) | ((g & 0xF8) << 2) | ((b & 0xF8) >> 3))

/* TFT_eSPI register init parameter list */
static const uint8_t ST7789_Init_List[] = {
        9,
        TFT_SWRST, TFT_INIT_DELAY, 255,
        TFT_SLPOUT, TFT_INIT_DELAY, 255, //exit sleep mode
        TFT_COLMOD, 1 + TFT_INIT_DELAY, ST7789_ColorMode_16bit, 10, //颜色模式
        TFT_MADCTL, 1, 0x00, //刷屏方向
        TFT_CASET, 4, 0x00, 0x00, 0x00, 0xF0, //240
        TFT_PASET, 4, 0x00, 0x00, 0x00, 0xF0, //240
        TFT_INVON, TFT_INIT_DELAY, 10, //Inversion ON
        TFT_NORON, TFT_INIT_DELAY, 10, //Normal display on, no args, w/delay
        TFT_DISPON, TFT_INIT_DELAY, 10            //Main screen turn on, no args, w/delay
};

#undef pgm_read_byte
#define pgm_read_byte(addr)   (*(uint8_t *)(addr))

#undef pgm_read_word
//#define pgm_read_word(addr) ({ typeof(addr) _addr = (addr); *(const unsigned short *)(_addr); })
#define pgm_read_word(addr) ( *(const unsigned short *)(addr))

#undef pgm_read_dword
//#define pgm_read_dword(addr) ({ typeof(addr) _addr = (addr); *(const unsigned long *)(_addr); })
#define pgm_read_dword(addr) ( *(const unsigned long *)(addr))

class TFT_eSPI {
public:
    TFT_eSPI(int16_t _w = TFT_WIDTH, int16_t _h = TFT_HEIGHT);

    TFT_FuncCallback writeCmd_8;
    TFT_FuncCallback writeData_8;

    void init(TFT_FuncCallback writeCmd, TFT_FuncCallback writeData);
    void             Hard_Reset(void),
                     BackLight(uint8_t bl_set),
                     resetViewport(void);

    void DELAY(uint16_t ms);

    void setRotation(uint8_t r); // Set the display image orientation to 0, 1, 2 or 3
    uint8_t getRotation(void); // Read the current rotation

    void commandList(const uint8_t *addr),
    //writeCmd_8(uint8_t CMD),
    //writeData_8(uint8_t Data),
         writeData_16(uint16_t Data),
         writeData_16S(uint16_t Data),
         writeData_32(uint32_t Data); /* Don't use , Temporary doesnot support */

    // These are virtual so the TFT_eSprite class can override them with sprite specific functions
    virtual void drawPixel(int32_t x, int32_t y, uint32_t color),
                 drawChar(int32_t x, int32_t y, uint16_t c, uint32_t TextColor, uint32_t BgColor, uint8_t size),
                 drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color),
                 drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color),
                 drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color);
    void drawNumber(int32_t x, int32_t y, uint32_t number, uint8_t size);

    virtual int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font),
                    drawChar(uint16_t uniCode, int32_t x, int32_t y),
                    getWidth(void), //get screen width
                    getHeight(void); //get screen height

    void invertDisplay(bool i); // Tell TFT to invert all displayed colours

    // Graphics drawing
    void fillScreen(uint32_t color),
         fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color),
         drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

    // The TFT_eSprite class inherits the following functions (not all are useful to Sprite class
    void setAddrWindow(int32_t x0, int32_t y0, int32_t w, int32_t h), // Note: start coordinates + width and height
         setWindow(int32_t x0, int32_t y0, int32_t x1, int32_t y1); // Note: start + end coordinates


    // Image rendering
    // Swap the byte order for pushImage() and pushPixels() - corrects endianness
    void setSwapBytes(bool swap);
    bool getSwapBytes(void);

    // Write a solid block of a single colour
    void pushBlock(uint16_t color, uint32_t len);
    // Write a set of pixels stored in memory, use setSwapBytes(true/false) function to correct endianess
    void pushPixels(const void *data_in, uint32_t len);
    void pushPixels_8bit(const void *data_in, uint32_t len);
    void pushPixels_2bit(const void *data_in, uint32_t len);

    // These are used to render images or sprites stored in RAM arrays (used by Sprite class for 16bpp Sprites)
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data, uint16_t transparent);
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data);

    // These are used to render images stored in FLASH (PROGMEM)
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data);
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, uint16_t transparent);

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data);
    // These are used by Sprite class pushSprite() member function for 1, 4 and 8 bits per pixel (bpp) colours
    // They are not intended to be used with user sketches (but could be)
    // Set bpp8 true for 8bpp sprites, false otherwise. The cmap pointer must be specified for 4bpp
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data, bool bpp8, uint16_t *cmap = nullptr);
    // FLASH version
    void
    pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data, bool bpp8, uint16_t *cmap = nullptr);

    void print(const char *string),
         print(int32_t x, int32_t y, const char *string),
         print(int32_t x, int32_t y, const char *string, uint32_t TextColor, uint32_t BgColor, uint8_t size);

    // Text rendering - value returned is the pixel width of the rendered text
    int16_t drawNumber(long intNumber, int32_t x, int32_t y, uint8_t font), // Draw integer using specified font number
            drawNumber(long intNumber, int32_t x, int32_t y), // Draw integer using current font

    // Decimal is the number of decimal places to render
    // Use with setTextDatum() to position values on TFT, and setTextPadding() to blank old displayed values
    drawFloat(float floatNumber, uint8_t decimal, int32_t x, int32_t y,
              uint8_t font), // Draw float using specified font number
    drawFloat(float floatNumber, uint8_t decimal, int32_t x, int32_t y), // Draw float using current font

    // Handle char arrays
    // Use with setTextDatum() to position string on TFT, and setTextPadding() to blank old displayed strings
    drawString(const char *string, int32_t x, int32_t y), // Draw string using current font
    drawString(const char *string, int32_t x, int32_t y, uint8_t font); // Draw string using specified font number

#ifdef LOAD_GFXFF
    void     setFreeFont(const GFXfont *f = nullptr), // Select the GFX Free Font
                setTextFont(uint8_t font); // Set the font number to use in future
#else
    void setFreeFont(uint8_t font), // Not used, historical fix to prevent an error
         setTextFont(uint8_t font); // Set the font number to use in future
#endif
    int16_t textWidth(const char *string), // Returns pixel width of string in current font
            textWidth(const char *string, uint8_t font), // Returns pixel width of string in specified font
            fontHeight(int16_t font), // Returns pixel height of string in specified font
            fontHeight(void); // Returns pixel width of string in current font

    void setTextColor(uint16_t color), // Set character (glyph) color only (background not over-written)
         setTextColor(uint16_t fgcolor, uint16_t bgcolor), // Set character (glyph) foreground and backgorund colour
         setTextSize(uint8_t size); // Set character size multiplier (this increases pixel size)

    void setTextDatum(uint8_t d); // Set text datum position (default is top left), see Section 6 above
    uint8_t getTextDatum(void);

    // Used by library and Smooth font class to extract Unicode point codes from a UTF8 encoded string
    uint16_t decodeUTF8(uint8_t *buf, uint16_t *index, uint16_t remaining),
             decodeUTF8(uint8_t c);

    uint32_t textcolor, textbgcolor; // Text foreground and background colours
    uint32_t bitmap_fg, bitmap_bg; // Bitmap foreground (bit=1) and background (bit=0) colours
    uint8_t  textfont, // Current selected font number
             textsize, // Current font size multiplier
             textdatum, // Text reference datum
             rotation; // Display rotation (0-3)

    uint8_t  decoderState = 0; // UTF8 decoder state        - not for user access
    uint16_t decoderBuffer; // Unicode code-point buffer - not for user access

protected:

    int32_t _init_width, _init_height; // Display w/h as input, used by setRotation()
    int32_t _width, _height; // Display w/h as modified by current rotation
    int32_t addr_row, addr_col; // Window position - used to minimise window commands

    int16_t _xPivot; // TFT x pivot point coordinate for rotated Sprites
    int16_t _yPivot; // TFT x pivot point coordinate for rotated Sprites

    // Viewport variables
    int32_t _vpX, _vpY, _vpW, _vpH; // Note: x start, y start, x end + 1, y end + 1
    int32_t _xDatum;
    int32_t _yDatum;
    int32_t _xWidth;
    int32_t _yHeight;
    bool    _vpDatum;
    bool    _vpOoB;

    int32_t cursor_x, cursor_y, padX; // Text cursor x,y and padding setting

    uint32_t fontsloaded; // Bit field of fonts loaded

    uint8_t glyph_ab, // Smooth font glyph delta Y (height) above baseline
            glyph_bb; // Smooth font glyph delta Y (height) below baseline

    bool isDigits; // adjust bounding box for numbers to reduce visual jiggling
    bool textwrapX, textwrapY; // If set, 'wrap' text at right and optionally bottom edge of display
    bool _swapBytes; // Swap the byte order for TFT pushImage()
    bool locked, inTransaction, lockTransaction; // SPI transaction and mutex lock flags
    bool _booted; // init() or begin() has already run once

    // User sketch manages these via set/getAttribute()
    bool _cp437; // If set, use correct CP437 charset (default is ON)
    bool _utf8; // If set, use UTF-8 decoder in print stream 'write()' function (default ON)
    bool _psram_enable; // Enable PSRAM use for library functions (TBD) and Sprites

    uint32_t _lastColor; // Buffered value of last colour used

#ifdef LOAD_GFXFF
    GFXfont  *gfxFont;
#endif

private:
    // Display variant settings
    uint8_t tabcolor, // ST7735 screen protector "tab" colour (now invalid)
            colstart = 0, rowstart = 0; // Screen display area to CGRAM area coordinate offsets




    //	gpio_port_t CS;
    //	gpio_port_t DC;
    //	gpio_port_t BL;
    //	gpio_port_t RST;

    /***************************************************************************************
    **                         Section 9: TFT_eSPI class conditional extensions
    ***************************************************************************************/
    // Load the Touch extension
#ifdef TOUCH_CS
#include "Extensions/Touch.h"        // Loaded if TOUCH_CS is defined by user
#endif

    // Load the Anti-aliased font extension
#ifdef SMOOTH_FONT
#include "Extensions/Smooth_font.h"  // Loaded if SMOOTH_FONT is defined by user
#endif

}; // End of class TFT_eSPI

/***************************************************************************************
**                         Section 10: Additional extension classes
***************************************************************************************/
// Load the Button Class
//#include "Extensions/Button.h"

// Load the Sprite Class
//#include "Extensions/Sprite.h"

//#ifdef __cplusplus
//}    /*__cplusplus*/
//#endif

#endif    /*ends #ifndef __TFT_LCD_H*/
