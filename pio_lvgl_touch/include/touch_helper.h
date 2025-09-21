#pragma once

#include <stdint.h>

void touch_helper_init(int16_t width, int16_t height, uint8_t rotation);
void touch_helper_poll(uint16_t &x, uint16_t &y, bool &pressed);
