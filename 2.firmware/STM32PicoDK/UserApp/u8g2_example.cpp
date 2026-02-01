//
// Created by moon on 2024/11/6.
//
#include "common_inc.h"
#include "OLed.h"
#include "Commander/Commander.h"
#include "SoftTimer/SoftTimer.h"

OLed      u8g2;
Commander commander(256);

//target variable
float target_velocity = 0;
char  str[32]         = "你好世界";

void receivedCommand(uint8_t *data, uint32_t size) {
    return ;
    commander.write((char *) data, size);
}
//
void doTarget(char *cmd) {
    memset(str, 0, 32);
    commander.str(str, cmd);
    u8g2.firstPage();
    do {
        u8g2.drawUTF8((u8g2.getWidth() - u8g2.getUTF8Width(str)) / 2, 32, str);
    } while (u8g2.nextPage());
}



#define SUN	0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4

void drawWeatherSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
    // fonts used:
    // u8g2_font_open_iconic_embedded_6x_t
    // u8g2_font_open_iconic_weather_6x_t
    // encoding values, see: https://github.com/olikraus/u8g2/wiki/fntgrpiconic

    switch(symbol)
    {
    case SUN:
        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
        u8g2.drawGlyph(x, y, 69);
        break;
    case SUN_CLOUD:
        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
        u8g2.drawGlyph(x, y, 65);
        break;
    case CLOUD:
        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
        u8g2.drawGlyph(x, y, 64);
        break;
    case RAIN:
        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
        u8g2.drawGlyph(x, y, 67);
        break;
    case THUNDER:
        u8g2.setFont(u8g2_font_open_iconic_embedded_6x_t);
        u8g2.drawGlyph(x, y, 67);
        break;
    }
}

void drawWeather(uint8_t symbol, int degree)
{
    drawWeatherSymbol(0, 48, symbol);
    u8g2.setFont(u8g2_font_logisoso32_tf);
    u8g2.setCursor(48+3, 42);

    u8g2.cursor_offset = 0;
    u8g2.print(degree);
    u8g2.print("°C");		// requires enableUTF8Print()
}

/*
  Draw a string with specified pixel offset.
  The offset can be negative.
  Limitation: The monochrome font with 8 pixel per glyph
*/
void drawScrollString(int16_t offset, const char *s)
{
    static char buf[36];	// should for screen with up to 256 pixel width
    size_t len;
    size_t char_offset = 0;
    u8g2_uint_t dx = 0;
    size_t visible = 0;
    len = strlen(s);
    if ( offset < 0 )
    {
        char_offset = (-offset)/8;
        dx = offset + char_offset*8;
        if ( char_offset >= u8g2.getDisplayWidth()/8 )
            return;
        visible = u8g2.getDisplayWidth()/8-char_offset+1;
        strncpy(buf, s, visible);
        buf[visible] = '\0';
        u8g2.setFont(u8g2_font_8x13_mf);
        u8g2.drawStr(char_offset*8-dx, 62, buf);
    }
    else
    {
        char_offset = offset / 8;
        if ( char_offset >= len )
            return;	// nothing visible
        dx = offset - char_offset*8;
        visible = len - char_offset;
        if ( visible > u8g2.getDisplayWidth()/8+1 )
            visible = u8g2.getDisplayWidth()/8+1;
        strncpy(buf, s+char_offset, visible);
        buf[visible] = '\0';
        u8g2.setFont(u8g2_font_8x13_mf);
        u8g2.drawStr(-dx, 62, buf);
    }

}

void draw(const char *s, uint8_t symbol, int degree)
{
    int16_t offset = -(int16_t)u8g2.getDisplayWidth();
    int16_t len = strlen(s);
    for(;;)
    {
        u8g2.firstPage();
        do {
            drawWeather(symbol, degree);
            drawScrollString(offset, s);
        } while ( u8g2.nextPage() );
        _delay(20);
        offset+=2;
        if ( offset > len*8+1 )
            break;
    }
}


void u8g2_weather_example(void) {
    u8g2.enable();
    _delay(100);

    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_wqy15_t_chinese3);

    u8g2.firstPage();
    do{
        u8g2.drawUTF8(0, 32, str);
    }while(u8g2.nextPage());

//    commander.add("str", doTarget);

    for(;;) {
        draw("What a beautiful day!", SUN, 27);
        draw("The sun's come out!", SUN_CLOUD, 19);
        draw("It's raining cats and dogs.", RAIN, 8);
        draw("That sounds like thunder.", THUNDER, 12);
        draw("It's stopped raining", CLOUD, 15);

//        commander.run();
    }

}