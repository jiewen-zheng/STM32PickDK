//
// Created by moon on 2023/11/4.
//

#include "common_inc.h"
#include "stm32f1xx.h"

#define CNT_BASE (72000000 / 1000)

uint32_t _micros() {
    uint32_t ms, cycle_cnt;

    do {
        ms        = HAL_GetTick();
        cycle_cnt = SysTick->VAL;
    } while (ms != HAL_GetTick());

    return (ms * 1000) + ((cycle_cnt / CNT_BASE) * 1000);
}

uint32_t _millis() {
    return HAL_GetTick();
}

void _delayUs(uint32_t us) {
    uint32_t start = _micros();

    while (_micros() - start < us) {
        asm volatile ("nop");
    }
}

void _delay(uint32_t ms) {
    HAL_Delay(ms - 1);
}
