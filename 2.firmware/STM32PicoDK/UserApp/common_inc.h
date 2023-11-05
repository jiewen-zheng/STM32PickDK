//
// Created by moon on 2023/11/5.
//

#ifndef STM32PICODK_COMMON_INC_H
#define STM32PICODK_COMMON_INC_H


#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"

void receivedCommand(uint8_t *data, uint32_t size);

uint32_t _micros();
uint32_t _millis();
void _delayUs(uint32_t us);
void _delay(uint32_t ms);

void Main();

#ifdef __cplusplus
}
#endif

#endif //STM32PICODK_COMMON_INC_H
