#include "touch_helper.h"

#include <Arduino.h>
#include <Wire.h>

#define TOUCH_MODULES_GT911

#include <TouchLib.h>
#include <REG/GT911Constants.h>

namespace {
constexpr int TOUCH_SCL_PIN = 8;
constexpr int TOUCH_SDA_PIN = 7;
constexpr uint8_t TOUCH_MODULE_ADDR = GT911_SLAVE_ADDRESS1;

// GT911 の生データ範囲（Arduino_GFX TouchCalibration の結果を反映）
constexpr int16_t RAW_MIN_X = 0;
constexpr int16_t RAW_MAX_X = 800;
constexpr int16_t RAW_MIN_Y = 0;
constexpr int16_t RAW_MAX_Y = 500;

bool touch_swap_xy = false;
int16_t touch_map_x1 = -1;
int16_t touch_map_x2 = -1;
int16_t touch_map_y1 = -1;
int16_t touch_map_y2 = -1;

int16_t touch_max_x = 0;
int16_t touch_max_y = 0;
uint16_t last_x = 0;
uint16_t last_y = 0;

TouchLib touch(Wire, TOUCH_SDA_PIN, TOUCH_SCL_PIN, TOUCH_MODULE_ADDR);

void translate_touch_raw(int16_t raw_x, int16_t raw_y)
{
    int32_t mapped_x = 0;
    int32_t mapped_y = 0;

    if (touch_swap_xy) {
        mapped_x = map(raw_y, touch_map_x1, touch_map_x2, 0, touch_max_x);
        mapped_y = map(raw_x, touch_map_y1, touch_map_y2, 0, touch_max_y);
    } else {
        mapped_x = map(raw_x, touch_map_x1, touch_map_x2, 0, touch_max_x);
        mapped_y = map(raw_y, touch_map_y1, touch_map_y2, 0, touch_max_y);
    }

    mapped_x = constrain(mapped_x, 0, touch_max_x);
    mapped_y = constrain(mapped_y, 0, touch_max_y);

    last_x = static_cast<uint16_t>(mapped_x);
    last_y = static_cast<uint16_t>(mapped_y);
}

void setup_axis_mapping(uint8_t rotation)
{
    switch (rotation) {
    case 3:
        touch_swap_xy = true;
        touch_map_x1 = RAW_MAX_X;
        touch_map_x2 = RAW_MIN_X;
        touch_map_y1 = RAW_MIN_Y;
        touch_map_y2 = RAW_MAX_Y;
        break;
    case 2:
        touch_swap_xy = false;
        touch_map_x1 = RAW_MAX_X;
        touch_map_x2 = RAW_MIN_X;
        touch_map_y1 = RAW_MAX_Y;
        touch_map_y2 = RAW_MIN_Y;
        break;
    case 1:
        touch_swap_xy = true;
        touch_map_x1 = RAW_MIN_Y;
        touch_map_x2 = RAW_MAX_Y;
        touch_map_y1 = RAW_MAX_X;
        touch_map_y2 = RAW_MIN_X;
        break;
    default:
        touch_swap_xy = false;
        touch_map_x1 = RAW_MIN_X;
        touch_map_x2 = RAW_MAX_X;
        touch_map_y1 = RAW_MIN_Y;
        touch_map_y2 = RAW_MAX_Y;
        break;
    }
}
} // namespace

void touch_helper_init(int16_t width, int16_t height, uint8_t rotation)
{
    touch_max_x = width - 1;
    touch_max_y = height - 1;

    setup_axis_mapping(rotation);

    Wire.begin(TOUCH_SDA_PIN, TOUCH_SCL_PIN);
    touch.init();
    touch.setRotation(rotation);
}

void touch_helper_poll(uint16_t &x, uint16_t &y, bool &pressed)
{
    pressed = false;

    if (touch.read()) {
        const uint8_t point_num = touch.getPointNum();
        if (point_num > 0) {
            const TP_Point point = touch.getPoint(0);
            translate_touch_raw(point.x, point.y);
            pressed = true;
        }
    }

    x = last_x;
    y = last_y;
}
