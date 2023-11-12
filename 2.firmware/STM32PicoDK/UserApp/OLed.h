//
// Created by moon on 2023/11/4.
//

#ifndef STM32PICODK_OLED_H
#define STM32PICODK_OLED_H

#include "U8g2/src/U8g2lib.h"

class OLed : public U8G2 {
public:
    int cursor_offset = 0;
public:
    explicit OLed(const u8g2_cb_t *rotation = U8G2_R0);

    void enable(bool en = true);

    void print(const char *msg);
    void print(int msg);

private:

};

#endif //STM32PICODK_OLED_H
