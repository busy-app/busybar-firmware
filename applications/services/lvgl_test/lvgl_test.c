#include <furi.h>

#include <ssd1320.h>

#include <lvgl.h>

#define TAG "LvglTestSrv"

#define COLOR_FORMAT     (LV_COLOR_FORMAT_L8)
#define BYTES_PER_PIXEL  (LV_COLOR_FORMAT_GET_SIZE(COLOR_FORMAT))
// TODO: Use partial draw to reduce memory usage?
#define DRAW_BUFFER_SIZE (SSD1320_W * SSD1320_H * BYTES_PER_PIXEL)
#define DISP_BUFFER_SIZE (SSD1320_BUF_SIZE)

#define TICK_PERIOD_MS (16)

typedef struct {
    FuriEventLoop* event_loop;
    uint8_t draw_buf[DRAW_BUFFER_SIZE];
    uint8_t disp_buf[DISP_BUFFER_SIZE];
} LvglTestSrv;

// TODO: Optimise conversion?
static void lvgl_test_l8_to_l4(const uint8_t* src, uint8_t* dst) {
    for(uint32_t i = 0; i < DISP_BUFFER_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst[i] = (src[draw_idx] >> 4) | (src[draw_idx + 1] & 0xF0);
    }
}

static void lvgl_flush_callback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map) {
    FURI_LOG_I(
        TAG, "Drawing area: (%ld, %ld), (%ld, %ld)", area->x1, area->y1, area->x2, area->y2);

    LvglTestSrv* instance = lv_display_get_user_data(display);

    lvgl_test_l8_to_l4(px_map, instance->disp_buf);
    ssd1320_draw(instance->disp_buf);
    lv_display_flush_ready(display);
}

static void lvgl_input_callback(lv_indev_t* indev, lv_indev_data_t* data) {
    UNUSED(indev);
    UNUSED(data);
    // TODO: Implement input
}

static void lvgl_log_callback(lv_log_level_t level, const char* buf) {
    char* line = strdup(buf);
    line[strlen(buf) - 1] = 0;

    switch(level) {
    case LV_LOG_LEVEL_INFO:
        FURI_LOG_I(TAG, "%s", line);
        break;
    case LV_LOG_LEVEL_WARN:
        FURI_LOG_W(TAG, "%s", line);
        break;
    case LV_LOG_LEVEL_ERROR:
        FURI_LOG_E(TAG, "%s", line);
        break;
    default:
        FURI_LOG_D(TAG, "%s", line);
    }

    free(line);
}

static void lvgl_test_srv_tick_callback(void* context) {
    UNUSED(context);
    lv_timer_periodic_handler();
}

static LvglTestSrv* lvgl_test_srv_alloc(void) {
    LvglTestSrv* instance = malloc(sizeof(LvglTestSrv));
    instance->event_loop = furi_event_loop_alloc();
    furi_event_loop_tick_set(
        instance->event_loop, TICK_PERIOD_MS, lvgl_test_srv_tick_callback, NULL);

    return instance;
}

static void lvgl_test_srv_init_lvgl(LvglTestSrv* instance) {
    lv_init();
    lv_tick_set_cb(furi_get_tick);
    lv_delay_set_cb(furi_delay_ms);
    lv_log_register_print_cb(lvgl_log_callback);

    lv_display_t* oled = lv_display_create(SSD1320_W, SSD1320_H);
    lv_display_set_user_data(oled, instance);
    lv_display_set_flush_cb(oled, lvgl_flush_callback);
    lv_display_set_color_format(oled, COLOR_FORMAT);
    lv_display_set_buffers(
        oled, instance->draw_buf, NULL, DRAW_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev, lvgl_input_callback);

    lv_obj_t* screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "Hello world!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

int lvgl_test_srv(void* arg) {
    UNUSED(arg);

    ssd1320_init();

    LvglTestSrv* instance = lvgl_test_srv_alloc();
    lvgl_test_srv_init_lvgl(instance);

    furi_event_loop_run(instance->event_loop);
    return 0;
}
