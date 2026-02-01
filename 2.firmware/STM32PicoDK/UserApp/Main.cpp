//
// Created by moon on 2023/11/4.
//

#include "common_inc.h"

#include "main.h"
#include "TFT_Lcd/tft_lcd.h"

void u8g2_weather_example(void);

extern "C" void Main() {
	u8g2_weather_example();

	for (;;) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		_delay(500);
	}
}
