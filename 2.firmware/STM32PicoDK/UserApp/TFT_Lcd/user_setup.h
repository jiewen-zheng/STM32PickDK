// ##################################################################################
//
// Section 1. Call up the right driver file and any options for it
//
// ##################################################################################

#ifndef __USER_SETUP_H
#define __USER_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif


// Only define one driver, the other ones must be commented out
//#define ILI9341_DRIVER       // Generic driver for common displays
//#define ILI9341_2_DRIVER     // Alternative ILI9341 driver, see https://github.com/Bodmer/TFT_eSPI/issues/1172
//#define ST7735_DRIVER      // Define additional parameters below for this display
//#define ILI9163_DRIVER     // Define additional parameters below for this display
//#define S6D02A1_DRIVER
//#define RPI_ILI9486_DRIVER // 20MHz maximum SPI
//#define HX8357D_DRIVER
//#define ILI9481_DRIVER
//#define ILI9486_DRIVER
//#define ILI9488_DRIVER     // WARNING: Do not connect ILI9488 display SDO to MISO if other devices share the SPI bus (TFT SDO does NOT tristate when CS is high)
//#define ST7789_DRIVER      // Full configuration option, define additional parameters below for this display
#define ST7789_2_DRIVER    // Minimal configuration option, define additional parameters below for this display
//#define R61581_DRIVER
//#define RM68140_DRIVER
//#define ST7796_DRIVER
//#define SSD1351_DRIVER
//#define SSD1963_480_DRIVER
//#define SSD1963_800_DRIVER
//#define SSD1963_800ALT_DRIVER
//#define ILI9225_DRIVER
//#define GC9A01_DRIVER


// ###### EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR STM32 SETUP ######

//#define TFT_DC			{GPIOB, GPIO_Pin_0}
//#define TFT_RST			{GPIOB, GPIO_Pin_1}
//#define TFT_BL			{GPIOB, GPIO_Pin_2}


// ##################################################################################
//
// Section 3. Define the fonts that are to be used here
//
// ##################################################################################

// Comment out the #defines below with // to stop that font being loaded
// The ESP8366 and ESP32 have plenty of memory so commenting out fonts is not
// normally necessary. If all fonts are loaded the extra FLASH space required is
// about 17Kbytes. To save FLASH space only enable the fonts you need!

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
//#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
//#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
//#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
//#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
//#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
//#define SMOOTH_FONT


/////////////////////////////////////////////////////////////////////////////////////
//                                                                                 //
//     DON'T TINKER WITH ANY OF THE FOLLOWING LINES, THESE ADD THE TFT DRIVERS     //
//       AND ESP8266 PIN DEFINITONS, THEY ARE HERE FOR BODMER'S CONVENIENCE!       //
//                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////


// Identical looking TFT displays may have a different colour ordering in the 16 bit colour
#define TFT_BGR 0   // Colour order Blue-Green-Red
#define TFT_RGB 1   // Colour order Red-Green-Blue


#define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
//#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

// Legacy setup support, RPI_DISPLAY_TYPE replaces RPI_DRIVER
#if defined (RPI_DRIVER)
#if !defined (RPI_DISPLAY_TYPE)
#define RPI_DISPLAY_TYPE
#endif
#endif

// Legacy setup support, RPI_ILI9486_DRIVER form is deprecated
// Instead define RPI_DISPLAY_TYPE and also define driver (e.g. ILI9486_DRIVER)
#if defined (RPI_ILI9486_DRIVER)
#if !defined (ILI9486_DRIVER)
#define ILI9486_DRIVER
#endif
#if !defined (RPI_DISPLAY_TYPE)
#define RPI_DISPLAY_TYPE
#endif
#endif

// Invoke 18 bit colour for selected displays
#if !defined (RPI_DISPLAY_TYPE) && !defined (TFT_PARALLEL_8_BIT) && !defined (ESP32_PARALLEL)
#if defined (ILI9481_DRIVER) || defined (ILI9486_DRIVER) || defined (ILI9488_DRIVER)
#define SPI_18BIT_DRIVER
#endif
#endif

// Load the right driver definition - do not tinker here !
#if   defined (ILI9341_DRIVER) || defined(ILI9341_2_DRIVER)
#include <TFT_Drivers/ILI9341_Defines.h>
#define  TFT_DRIVER 0x9341
#elif defined (ST7735_DRIVER)
#include <TFT_Drivers/ST7735_Defines.h>
#define  TFT_DRIVER 0x7735
#elif defined (ILI9163_DRIVER)
#include <TFT_Drivers/ILI9163_Defines.h>
#define  TFT_DRIVER 0x9163
#elif defined (S6D02A1_DRIVER)
#include <TFT_Drivers/S6D02A1_Defines.h>
#define  TFT_DRIVER 0x6D02
#elif defined (ST7796_DRIVER)
#include "TFT_Drivers/ST7796_Defines.h"
#define  TFT_DRIVER 0x7796
#elif defined (ILI9486_DRIVER)
#include <TFT_Drivers/ILI9486_Defines.h>
#define  TFT_DRIVER 0x9486
#elif defined (ILI9481_DRIVER)
#include <TFT_Drivers/ILI9481_Defines.h>
#define  TFT_DRIVER 0x9481
#elif defined (ILI9488_DRIVER)
#include <TFT_Drivers/ILI9488_Defines.h>
#define  TFT_DRIVER 0x9488
#elif defined (HX8357D_DRIVER)
#include "TFT_Drivers/HX8357D_Defines.h"
#define  TFT_DRIVER 0x8357
#elif defined (EPD_DRIVER)
#include "TFT_Drivers/EPD_Defines.h"
#define  TFT_DRIVER 0xE9D
#elif defined (ST7789_DRIVER)
#include "TFT_Drivers/ST7789_Defines.h"
#define  TFT_DRIVER 0x7789
#elif defined (R61581_DRIVER)
#include "TFT_Drivers/R61581_Defines.h"
#define  TFT_DRIVER 0x6158
#elif defined (ST7789_2_DRIVER)
#include "TFT_Drivers/ST7789_2_Defines.h"
#define  TFT_DRIVER 0x778B
#elif defined (RM68140_DRIVER)
#include "TFT_Drivers/RM68140_Defines.h"
#define  TFT_DRIVER 0x6814
#elif defined (SSD1351_DRIVER)
#include "TFT_Drivers/SSD1351_Defines.h"
#define  TFT_DRIVER 0x1351
#elif defined (SSD1963_480_DRIVER)
#include "TFT_Drivers/SSD1963_Defines.h"
#define  TFT_DRIVER 0x1963
#elif defined (SSD1963_800_DRIVER)
#include "TFT_Drivers/SSD1963_Defines.h"
#define  TFT_DRIVER 0x1963
#elif defined (SSD1963_800ALT_DRIVER)
#include "TFT_Drivers/SSD1963_Defines.h"
#define  TFT_DRIVER 0x1963
#elif defined (SSD1963_800BD_DRIVER)
#include "TFT_Drivers/SSD1963_Defines.h"
#define  TFT_DRIVER 0x1963
#elif defined (GC9A01_DRIVER)
#include "TFT_Drivers/GC9A01_Defines.h"
#define  TFT_DRIVER 0x9A01
#elif defined (ILI9225_DRIVER)
#include "TFT_Drivers/ILI9225_Defines.h"
#define  TFT_DRIVER 0x9225
                              // <<<<<<<<<<<<<<<<<<<<<<<< ADD NEW DRIVER HERE
                              // XYZZY_init.h and XYZZY_rotation.h must also be added in TFT_eSPI.cpp
#elif defined (XYZZY_DRIVER)
#include "TFT_Drivers/XYZZY_Defines.h"
#define  TFT_DRIVER 0x0000
#else
#define  TFT_DRIVER 0x0000
#endif



#ifdef __cplusplus
}   // extern "C" {
#endif

#endif // __USER_SETUP_H