//
// Created by moon on 2023/11/4.
//

#include "OLed.h"
#include "common_inc.h"
#include "i2c.h"

#include <string>

extern "C" uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {

    static uint8_t buffer[32];        /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t        *data;

    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *) arg_ptr;
            while (arg_int > 0) {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
            break;

        case U8X8_MSG_BYTE_INIT:
//            if ( u8x8->bus_clock == 0 ) 	/* issue 769 */
//                u8x8->bus_clock = u8x8->display_info->i2c_bus_clock_100kHz * 100000UL;
            break;

        case U8X8_MSG_BYTE_SET_DC:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            HAL_I2C_Master_Transmit(&hi2c2, u8x8_GetI2CAddress(u8x8), buffer, buf_idx, 1000);
            break;

        default:
            return 0;
    }

    return 1;
}

extern "C" uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;

#ifndef __AVR__
            /* this case is not compiled for any AVR, because AVR uC are so slow */
            /* that this delay does not matter */
        case U8X8_MSG_DELAY_NANO:
            _delayUs(arg_int == 0 ? 0 : 1);
            break;
#endif

        case U8X8_MSG_DELAY_10MICRO:
            /* not used at the moment */
            break;

        case U8X8_MSG_DELAY_100NANO:
            /* not used at the moment */
            break;

        case U8X8_MSG_DELAY_MILLI:
            _delay(arg_int);
            break;

        case U8X8_MSG_DELAY_I2C:
            /* arg_int is 1 or 4: 100KHz (5us) or 400KHz (1.25us) */
//            _delayUs(arg_int <= 2 ? 2 : 5);
            break;
        case U8X8_MSG_GPIO_I2C_CLOCK:
        case U8X8_MSG_GPIO_I2C_DATA:
            break;

        default:
            return 0;
    }
    return 1;
}

OLed::OLed(const u8g2_cb_t *rotation) {
    u8g2_Setup_ssd1312_i2c_128x64_noname_f(&u8g2, rotation, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
}

void OLed::enable(bool en) {
    en ?
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET) :
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET);
}

void OLed::print(const char *msg) {
    uint16_t x = (cursor_offset == 0) ? getCursorX() : cursor_offset + getCursorX();

    drawUTF8(x, getCursorY(), msg);

    cursor_offset += getUTF8Width(msg);
}

void OLed::print(int msg) {
    uint16_t x = (cursor_offset == 0) ? getCursorX() : cursor_offset;
    const char *str = std::to_string(msg).c_str();

    drawStr(x, getCursorY(), str);

    cursor_offset += getUTF8Width(str);
}

