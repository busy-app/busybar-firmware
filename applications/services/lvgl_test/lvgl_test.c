#include <furi.h>
#include <lvgl.h>

#include <ssd1320.h>
#include <led_display/led_display.h>

#define TAG "LvglTestSrv"

#define FRONT_W                (DOT_MATRIX_W)
#define FRONT_H                (DOT_MATRIX_H)
#define FRONT_COLOR_FORMAT     (LV_COLOR_FORMAT_RGB888)
#define FRONT_BYTES_PER_PIXEL  (LV_COLOR_FORMAT_GET_SIZE(FRONT_COLOR_FORMAT))
#define FRONT_DRAW_BUFFER_SIZE (FRONT_W * FRONT_H * FRONT_BYTES_PER_PIXEL)

#define BACK_W                 (SSD1320_W)
#define BACK_H                 (SSD1320_H)
#define BACK_COLOR_FORMAT      (LV_COLOR_FORMAT_L8)
#define BACK_BYTES_PER_PIXEL   (LV_COLOR_FORMAT_GET_SIZE(BACK_COLOR_FORMAT))
// TODO: Use partial draw to reduce memory usage?
#define BACK_DRAW_BUFFER_SIZE  (BACK_W * BACK_H * BACK_BYTES_PER_PIXEL)
#define BACK_FRAME_BUFFER_SIZE (SSD1320_BUF_SIZE)

#define TICK_PERIOD_MS (16)

typedef enum {
    DisplayTypeFront,
    DisplayTypeBack,
    DisplayTypeMax,
} DisplayType;

typedef struct {
    lv_display_t* display;
    uint8_t* draw_buffer;
    uint8_t* frame_buffer;
} DisplayData;

typedef struct {
    FuriEventLoop* event_loop;
    DotMatrixSrv* dot_matrix;
    DisplayData display_data[DisplayTypeMax];
} LvglTestSrv;

// TODO: Optimise conversion?
static void lvgl_test_l8_to_l4(uint8_t* dst, const uint8_t* src) {
    for(uint32_t i = 0; i < BACK_FRAME_BUFFER_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst[i] = (src[draw_idx] >> 4) | (src[draw_idx + 1] & 0xF0);
    }
}

static DisplayType
    lvgl_test_srv_get_display_type(LvglTestSrv* instance, const lv_display_t* display) {
    DisplayType display_type;

    for(display_type = 0; display_type < DisplayTypeMax; ++display_type) {
        if(instance->display_data[display_type].display == display) {
            break;
        }
    }

    return display_type;
}

static void lvgl_flush_callback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map) {
    FURI_LOG_I(
        TAG, "Drawing area: (%ld, %ld), (%ld, %ld)", area->x1, area->y1, area->x2, area->y2);

    LvglTestSrv* instance = lv_display_get_user_data(display);

    const DisplayType display_type = lvgl_test_srv_get_display_type(instance, display);
    furi_check(display_type < DisplayTypeMax);

    DisplayData* display_data = &instance->display_data[display_type];
    furi_check(px_map == display_data->draw_buffer);

    if(display_type == DisplayTypeFront) {
        dot_matrix_draw(instance->dot_matrix, display_data->draw_buffer);
    } else if(display_type == DisplayTypeBack) {
        lvgl_test_l8_to_l4(display_data->frame_buffer, display_data->draw_buffer);
        ssd1320_draw(display_data->frame_buffer);
    }

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

static void lvgl_test_srv_init_front(LvglTestSrv* instance) {
    DisplayData* display_data = &instance->display_data[DisplayTypeFront];
    display_data->draw_buffer = malloc(FRONT_DRAW_BUFFER_SIZE);

    lv_display_t* front = lv_display_create(FRONT_W, FRONT_H);
    lv_display_set_user_data(front, instance);
    lv_display_set_flush_cb(front, lvgl_flush_callback);
    lv_display_set_color_format(front, FRONT_COLOR_FORMAT);
    lv_display_set_buffers(
        front,
        display_data->draw_buffer,
        NULL,
        FRONT_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);

    display_data->display = front;

    lv_obj_t* screen = lv_display_get_screen_active(front);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "Hello there");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static void lvgl_test_srv_init_back(LvglTestSrv* instance) {
    ssd1320_init();

    DisplayData* display_data = &instance->display_data[DisplayTypeBack];
    display_data->draw_buffer = malloc(BACK_DRAW_BUFFER_SIZE);
    display_data->frame_buffer = malloc(BACK_FRAME_BUFFER_SIZE);

    lv_display_t* back = lv_display_create(BACK_W, BACK_H);
    lv_display_set_user_data(back, instance);
    lv_display_set_flush_cb(back, lvgl_flush_callback);
    lv_display_set_color_format(back, BACK_COLOR_FORMAT);
    lv_display_set_buffers(
        back,
        display_data->draw_buffer,
        NULL,
        BACK_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);

    display_data->display = back;

    lv_obj_t* screen = lv_display_get_screen_active(back);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "General Kenobi");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static LvglTestSrv* lvgl_test_srv_alloc(void) {
    LvglTestSrv* instance = malloc(sizeof(LvglTestSrv));
    instance->event_loop = furi_event_loop_alloc();
    instance->dot_matrix = furi_record_open(RECORD_DOT_MATRIX);

    furi_event_loop_tick_set(
        instance->event_loop, TICK_PERIOD_MS, lvgl_test_srv_tick_callback, NULL);

    lv_init();
    lv_tick_set_cb(furi_get_tick);
    lv_delay_set_cb(furi_delay_ms);
    lv_log_register_print_cb(lvgl_log_callback);

    lvgl_test_srv_init_front(instance);
    lvgl_test_srv_init_back(instance);

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev, lvgl_input_callback);

    return instance;
}

int lvgl_test_srv(void* arg) {
    UNUSED(arg);

    LvglTestSrv* instance = lvgl_test_srv_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
