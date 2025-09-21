#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include "esp_heap_caps.h"

#define JC1060P470
#define GFX_DEV_DEVICE JC1060P470
#define GFX_BL 23
#define DSI_PANEL

static constexpr int32_t SCREEN_HOR_RES = 1024;
static constexpr int32_t SCREEN_VER_RES = 600;
static constexpr size_t LVGL_BUFFER_LINES = 40;  // adjust to balance FPS vs RAM

static Arduino_ESP32DSIPanel *dsipanel = new Arduino_ESP32DSIPanel(
    40, 160, 160,
    10, 23, 12,
    48000000);

static Arduino_DSI_Display *gfx = new Arduino_DSI_Display(
    SCREEN_HOR_RES, SCREEN_VER_RES, dsipanel, 0, true,
    27, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t *lv_buf_1 = nullptr;
static lv_color_t *lv_buf_2 = nullptr;

static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    const int32_t x = area->x1;
    const int32_t y = area->y1;
    const int32_t w = area->x2 - area->x1 + 1;
    const int32_t h = area->y2 - area->y1 + 1;

    gfx->draw16bitRGBBitmap(x, y, reinterpret_cast<uint16_t *>(color_p), w, h);
    lv_disp_flush_ready(disp);
}

static void lvgl_port_init()
{
    lv_init();

    const size_t buf_pixels = SCREEN_HOR_RES * LVGL_BUFFER_LINES;
    lv_buf_1 = static_cast<lv_color_t *>(heap_caps_malloc(buf_pixels * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    lv_buf_2 = static_cast<lv_color_t *>(heap_caps_malloc(buf_pixels * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));

    if (!lv_buf_1 || !lv_buf_2) {
        Serial.println("[LVGL] Failed to allocate draw buffers in PSRAM");
        while (true) {
            delay(1000);
        }
    }

    lv_disp_draw_buf_init(&draw_buf, lv_buf_1, lv_buf_2, buf_pixels);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_HOR_RES;
    disp_drv.ver_res = SCREEN_VER_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.sw_rotate = 0;
    lv_disp_drv_register(&disp_drv);
}

static void create_demo_ui()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Helo from LVGL!");
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void setup()
{
#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    Serial.begin(115200);
    Serial.println("LVGL demo starting...");

    gfx->begin();
    gfx->setRotation(0);
    gfx->fillScreen(WHITE);

    lvgl_port_init();
    create_demo_ui();
}

void loop()
{
    lv_tick_inc(5);
    lv_timer_handler();
    delay(5);
}
