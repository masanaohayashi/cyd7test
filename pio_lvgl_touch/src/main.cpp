#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include "esp_heap_caps.h"
#include "touch_helper.h"

#include <cstdint>

#define JC1060P470
#define GFX_DEV_DEVICE JC1060P470
#define GFX_BL 23
#define DSI_PANEL

static constexpr int32_t SCREEN_WIDTH = 1024;
static constexpr int32_t SCREEN_HEIGHT = 600;
static constexpr size_t LVGL_BUFFER_LINES = 40;  // 1024 * 40 * 2 bytes ≈ 80 KB

static Arduino_ESP32DSIPanel *dsipanel = new Arduino_ESP32DSIPanel(
    40, 160, 160,
    10, 23, 12,
    48000000);

static Arduino_DSI_Display *gfx = new Arduino_DSI_Display(
    SCREEN_WIDTH, SCREEN_HEIGHT, dsipanel, 0, true,
    27, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;
static lv_color_t *lv_buf_1 = nullptr;
static lv_color_t *lv_buf_2 = nullptr;
static lv_obj_t *message_label = nullptr;

static void update_message_with_button(uint32_t index)
{
    if (message_label == nullptr) {
        return;
    }
    lv_label_set_text_fmt(message_label, "Button %lu pressed", static_cast<unsigned long>(index));
}

static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    const int32_t x = area->x1;
    const int32_t y = area->y1;
    const int32_t w = area->x2 - area->x1 + 1;
    const int32_t h = area->y2 - area->y1 + 1;

    gfx->draw16bitRGBBitmap(x, y, reinterpret_cast<uint16_t *>(color_p), w, h);
    lv_disp_flush_ready(disp);
}

static void lvgl_touchpad_read(lv_indev_drv_t * /*drv*/, lv_indev_data_t *data)
{
    uint16_t x = 0;
    uint16_t y = 0;
    bool pressed = false;

    touch_helper_poll(x, y, pressed);

    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = static_cast<lv_coord_t>(x);
    data->point.y = static_cast<lv_coord_t>(y);
}

static void lv_port_init()
{
    lv_init();

    const size_t buf_pixels = SCREEN_WIDTH * LVGL_BUFFER_LINES;
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
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

static void button_event_cb(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_SHORT_CLICKED) {
        return;
    }

    const uintptr_t index = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    update_message_with_button(static_cast<uint32_t>(index));
}

static void create_demo_ui()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x202020), LV_PART_MAIN);

    lv_obj_t *title_label = lv_label_create(scr);
    lv_label_set_text(title_label, "LVGL Touch Demo");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x32CD32), LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 30);

    static lv_coord_t col_dsc[] = {140, 140, 140, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {100, 100, 100, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(grid, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(grid, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_column(grid, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, -20);

    for (uint32_t index = 1; index <= 9; ++index) {
        lv_obj_t *btn = lv_btn_create(grid);
        const uint32_t row = (index - 1) / 3;
        const uint32_t col = (index - 1) % 3;
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_set_size(btn, 120, 80);
        lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(index)));

        lv_obj_t *btn_label = lv_label_create(btn);
        lv_label_set_text_fmt(btn_label, "%lu", static_cast<unsigned long>(index));
        lv_obj_center(btn_label);
    }

    lv_obj_t *message_container = lv_obj_create(scr);
    lv_obj_remove_style_all(message_container);
    lv_obj_set_size(message_container, SCREEN_WIDTH, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(message_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(message_container, 12, LV_PART_MAIN);
    lv_obj_align(message_container, LV_ALIGN_BOTTOM_MID, 0, -12);

    message_label = lv_label_create(message_container);
    lv_label_set_text(message_label, "Waiting for input...");
    lv_obj_set_style_text_color(message_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 0);
}

void setup()
{
#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    Serial.begin(115200);
    Serial.println("LVGL touch demo starting...");

    gfx->begin();
    gfx->setRotation(0);
    gfx->fillScreen(BLACK);

    touch_helper_init(SCREEN_WIDTH, SCREEN_HEIGHT, gfx->getRotation());

    lv_port_init();
    create_demo_ui();
}

void loop()
{
    lv_timer_handler();
    delay(5);
    lv_tick_inc(5);
}
