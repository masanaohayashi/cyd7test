#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <FS.h>
#include <SD_MMC.h>
#include <lvgl.h>
#include "esp_heap_caps.h"
#include "touch_helper.h"

#include <cstdint>
#include <cstdarg>

#define JC1060P470
#define GFX_DEV_DEVICE JC1060P470
#define GFX_BL 23
#define DSI_PANEL

namespace {
constexpr int32_t SCREEN_WIDTH = 1024;
constexpr int32_t SCREEN_HEIGHT = 600;
constexpr size_t LVGL_BUFFER_LINES = 40;
constexpr const char *MOUNT_POINT = "/sdcard";
constexpr const char *FILE_PATH = "/lvgl_sd_demo.txt";

Arduino_ESP32DSIPanel *dsipanel = new Arduino_ESP32DSIPanel(
    40, 160, 160,
    10, 23, 12,
    48000000);

Arduino_DSI_Display *gfx = new Arduino_DSI_Display(
    SCREEN_WIDTH, SCREEN_HEIGHT, dsipanel, 0, true,
    27, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));

lv_disp_draw_buf_t draw_buf;
lv_disp_drv_t disp_drv;
lv_indev_drv_t indev_drv;

lv_color_t *lv_buf_1 = nullptr;
lv_color_t *lv_buf_2 = nullptr;

lv_obj_t *message_label = nullptr;
bool sd_ready = false;

void update_message(const char *fmt, ...)
{
    if (message_label == nullptr) {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    lv_label_set_text(message_label, buffer);
}

bool mount_sd_if_needed()
{
    if (sd_ready) {
        return true;
    }

    if (!SD_MMC.begin(MOUNT_POINT, true)) {
        sd_ready = false;
        Serial.println("[SD] Mount failed");
        update_message("SD mount failed");
        return false;
    }

    sd_ready = true;
    Serial.println("[SD] Mount success");
    update_message("SD ready at %s", MOUNT_POINT);
    return true;
}

void write_file()
{
    if (!mount_sd_if_needed()) {
        return;
    }

    File file = SD_MMC.open(FILE_PATH, FILE_WRITE);
    if (!file) {
        update_message("Failed to open %s for write", FILE_PATH);
        Serial.println("[SD] Open for write failed");
        return;
    }

    const String payload = String("Hello from SD demo at ") + millis();
    const bool ok = file.println(payload);
    file.close();

    if (ok) {
        update_message("Wrote string to %s:\n\"%s\"", FILE_PATH, payload.c_str());
        Serial.printf("[SD] Write success: %s\n", payload.c_str());
    } else {
        update_message("Write error on %s", FILE_PATH);
        Serial.println("[SD] Write failed");
    }
}

void read_file()
{
    if (!mount_sd_if_needed()) {
        return;
    }

    if (!SD_MMC.exists(FILE_PATH)) {
        update_message("File not found: %s", FILE_PATH);
        Serial.println("[SD] File missing");
        return;
    }

    File file = SD_MMC.open(FILE_PATH, FILE_READ);
    if (!file) {
        update_message("Failed to open %s", FILE_PATH);
        Serial.println("[SD] Open for read failed");
        return;
    }

    String content;
    while (file.available() && content.length() < 512) {
        content += static_cast<char>(file.read());
    }
    file.close();
    content.trim();

    if (content.isEmpty()) {
        update_message("%s is empty", FILE_PATH);
        Serial.println("[SD] File empty");
    } else {
        update_message("Read string from %s:\n\"%s\"", FILE_PATH, content.c_str());
        Serial.printf("[SD] Read content: %s\n", content.c_str());
    }
}

void delete_file()
{
    if (!mount_sd_if_needed()) {
        return;
    }

    if (!SD_MMC.exists(FILE_PATH)) {
        update_message("Nothing to delete: %s", FILE_PATH);
        Serial.println("[SD] Delete skipped, file missing");
        return;
    }

    if (SD_MMC.remove(FILE_PATH)) {
        update_message("Deleted %s", FILE_PATH);
        Serial.println("[SD] File deleted");
    } else {
        update_message("Failed to delete %s", FILE_PATH);
        Serial.println("[SD] Delete failed");
    }
}

void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    const int32_t x = area->x1;
    const int32_t y = area->y1;
    const int32_t w = area->x2 - area->x1 + 1;
    const int32_t h = area->y2 - area->y1 + 1;

    gfx->draw16bitRGBBitmap(x, y, reinterpret_cast<uint16_t *>(color_p), w, h);
    lv_disp_flush_ready(disp);
}

void lvgl_touchpad_read(lv_indev_drv_t *, lv_indev_data_t *data)
{
    uint16_t x = 0;
    uint16_t y = 0;
    bool pressed = false;

    touch_helper_poll(x, y, pressed);

    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = static_cast<lv_coord_t>(x);
    data->point.y = static_cast<lv_coord_t>(y);
}

void lv_port_init()
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

enum class SdAction : uint8_t {
    Write = 1,
    Read,
    Delete
};

void button_event_cb(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_SHORT_CLICKED) {
        return;
    }

    const SdAction action = static_cast<SdAction>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));

    switch (action) {
    case SdAction::Write:
        write_file();
        break;
    case SdAction::Read:
        read_file();
        break;
    case SdAction::Delete:
        delete_file();
        break;
    }
}

void create_ui()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x202020), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "SD Card LVGL Demo");
    lv_obj_set_style_text_color(title, lv_color_hex(0x32CD32), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_t *button_row = lv_obj_create(scr);
    lv_obj_remove_style_all(button_row);
    lv_obj_set_size(button_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(button_row, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(button_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(button_row, 18, LV_PART_MAIN);
    lv_obj_align(button_row, LV_ALIGN_CENTER, 0, -40);

    const struct {
        const char *label;
        SdAction action;
    } buttons[] = {
        {"Write File", SdAction::Write},
        {"Read File", SdAction::Read},
        {"Delete File", SdAction::Delete},
    };

    for (const auto &btn_def : buttons) {
        lv_obj_t *btn = lv_btn_create(button_row);
        lv_obj_set_size(btn, 220, 90);
        lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_ALL, reinterpret_cast<void *>(static_cast<uintptr_t>(btn_def.action)));

        lv_obj_t *btn_label = lv_label_create(btn);
        lv_label_set_text(btn_label, btn_def.label);
        lv_obj_center(btn_label);
    }

    lv_obj_t *msg_container = lv_obj_create(scr);
    lv_obj_remove_style_all(msg_container);
    lv_obj_set_size(msg_container, SCREEN_WIDTH - 80, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(msg_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(msg_container, 16, LV_PART_MAIN);
    lv_obj_align(msg_container, LV_ALIGN_BOTTOM_MID, 0, -36);

    message_label = lv_label_create(msg_container);
    lv_label_set_text(message_label, "Waiting for SD action...");
    lv_obj_set_width(message_label, SCREEN_WIDTH - 120);
    lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

} // namespace

void setup()
{
#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    Serial.begin(115200);
    Serial.println("SD card LVGL demo starting...");

    gfx->begin();
    gfx->setRotation(0);
    gfx->fillScreen(BLACK);

    touch_helper_init(SCREEN_WIDTH, SCREEN_HEIGHT, gfx->getRotation());

    lv_port_init();
    create_ui();

    mount_sd_if_needed();
}

void loop()
{
    lv_timer_handler();
    lv_tick_inc(5);
    delay(5);
}
