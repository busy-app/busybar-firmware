#include "gui_lvgl.h"

#include <furi.h>
#include <lvgl.h>

#include <ssd1320.h>
#include <led_display/led_display.h>

#define TAG "GuiLvgl"

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

typedef struct {
    lv_display_t* lv_display;
    uint8_t* draw_buffer;
    uint8_t* frame_buffer;
} GuiDisplayData;

struct GuiLvgl {
    FuriEventLoop* event_loop;
    FuriMutex* access_mutex;
    DotMatrixSrv* dot_matrix;
    GuiDisplayData display_data[GuiDisplayIdMax];
};

// TODO: Optimise conversion?
static void gui_lvgl_l8_to_l4(uint8_t* dst, const uint8_t* src) {
    for(uint32_t i = 0; i < BACK_FRAME_BUFFER_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst[i] = (src[draw_idx] >> 4) | (src[draw_idx + 1] & 0xF0);
    }
}

static GuiDisplayId gui_lvlgl_get_display_id(GuiLvgl* instance, const lv_display_t* display) {
    GuiDisplayId display_type;

    for(display_type = 0; display_type < GuiDisplayIdMax; ++display_type) {
        if(instance->display_data[display_type].lv_display == display) {
            break;
        }
    }

    return display_type;
}

static void
    gui_lvgl_flush_callback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map) {
    FURI_LOG_D(
        TAG, "Drawing area: (%ld, %ld), (%ld, %ld)", area->x1, area->y1, area->x2, area->y2);

    GuiLvgl* instance = lv_display_get_user_data(display);

    const GuiDisplayId display_type = gui_lvlgl_get_display_id(instance, display);
    furi_check(display_type < GuiDisplayIdMax);

    GuiDisplayData* display_data = &instance->display_data[display_type];
    furi_check(px_map == display_data->draw_buffer);

    if(display_type == GuiDisplayIdFront) {
        dot_matrix_draw(instance->dot_matrix, display_data->draw_buffer);
    } else if(display_type == GuiDisplayIdBack) {
        gui_lvgl_l8_to_l4(display_data->frame_buffer, display_data->draw_buffer);
        ssd1320_draw(display_data->frame_buffer);
    }

    lv_display_flush_ready(display);
}

static void gui_lvgl_input_callback(lv_indev_t* indev, lv_indev_data_t* data) {
    UNUSED(indev);
    UNUSED(data);
    // TODO: Implement input
}

static void gui_lvgl_log_callback(lv_log_level_t level, const char* buf) {
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

static void gui_lvgl_tick_callback(void* context) {
    furi_assert(context);
    GuiLvgl* instance = context;

    if(furi_mutex_acquire(instance->access_mutex, 2) == FuriStatusOk) {
        lv_timer_periodic_handler();
        furi_mutex_release(instance->access_mutex);
    } else {
        FURI_LOG_W(TAG, "Gui lockup: tick skipped");
    }
}

static void gui_lvgl_init_front(GuiLvgl* instance) {
    GuiDisplayData* display_data = &instance->display_data[GuiDisplayIdFront];

    display_data->draw_buffer = malloc(FRONT_DRAW_BUFFER_SIZE);
    display_data->lv_display = lv_display_create(FRONT_W, FRONT_H);

    lv_display_set_user_data(display_data->lv_display, instance);
    lv_display_set_flush_cb(display_data->lv_display, gui_lvgl_flush_callback);
    lv_display_set_color_format(display_data->lv_display, FRONT_COLOR_FORMAT);
    lv_display_set_buffers(
        display_data->lv_display,
        display_data->draw_buffer,
        NULL,
        FRONT_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);
}

static void gui_lvgl_init_back(GuiLvgl* instance) {
    ssd1320_init();

    GuiDisplayData* display_data = &instance->display_data[GuiDisplayIdBack];

    display_data->draw_buffer = malloc(BACK_DRAW_BUFFER_SIZE);
    display_data->frame_buffer = malloc(BACK_FRAME_BUFFER_SIZE);
    display_data->lv_display = lv_display_create(BACK_W, BACK_H);

    lv_display_set_user_data(display_data->lv_display, instance);
    lv_display_set_flush_cb(display_data->lv_display, gui_lvgl_flush_callback);
    lv_display_set_color_format(display_data->lv_display, BACK_COLOR_FORMAT);
    lv_display_set_buffers(
        display_data->lv_display,
        display_data->draw_buffer,
        NULL,
        BACK_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);
}

static void gui_lvgl_init_input(GuiLvgl* instance) {
    lv_indev_t* encoder = lv_indev_create();
    lv_indev_set_user_data(encoder, instance);
    lv_indev_set_type(encoder, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(encoder, gui_lvgl_input_callback);
}

static void gui_lvgl_init_styles(GuiLvgl* instance) {
    for(uint32_t i = 0; i < GuiDisplayIdMax; ++i) {
        lv_display_t* display = instance->display_data[i].lv_display;
        lv_obj_t* active = lv_display_get_screen_active(display);

        lv_obj_set_style_bg_color(active, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_text_color(active, lv_color_white(), LV_PART_MAIN);
    }
}

static GuiLvgl* gui_lvgl_alloc(void) {
    GuiLvgl* instance = malloc(sizeof(GuiLvgl));

    instance->event_loop = furi_event_loop_alloc();
    instance->access_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->dot_matrix = furi_record_open(RECORD_DOT_MATRIX);

    furi_event_loop_tick_set(instance->event_loop, TICK_PERIOD_MS, gui_lvgl_tick_callback, instance);

    lv_init();
    lv_tick_set_cb(furi_get_tick);
    lv_delay_set_cb(furi_delay_ms);
    lv_log_register_print_cb(gui_lvgl_log_callback);

    gui_lvgl_init_front(instance);
    gui_lvgl_init_back(instance);
    gui_lvgl_init_input(instance);
    gui_lvgl_init_styles(instance);

    furi_record_create(RECORD_GUI_LVGL, instance);
    return instance;
}

int gui_lvgl_srv(void* arg) {
    UNUSED(arg);

    GuiLvgl* instance = gui_lvgl_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

// Public API functions

void gui_lvgl_acquire(GuiLvgl* instance) {
    furi_check(instance);
    furi_check(furi_mutex_acquire(instance->access_mutex, FuriWaitForever) == FuriStatusOk);
}

void gui_lvgl_release(GuiLvgl* instance) {
    furi_check(instance);
    furi_check(furi_mutex_release(instance->access_mutex) == FuriStatusOk);
}

lv_display_t* gui_lvgl_get_display(GuiLvgl* instance, GuiDisplayId display_id) {
    furi_check(instance);
    furi_check(display_id < GuiDisplayIdMax);
    furi_check(furi_mutex_get_owner(instance->access_mutex) == furi_thread_get_current_id());

    return instance->display_data[display_id].lv_display;
}

lv_obj_t* gui_lvgl_get_layer(GuiLvgl* instance, GuiDisplayId display_id, GuiLayerId layer_id) {
    furi_check(instance);
    furi_check(display_id < GuiDisplayIdMax);
    furi_check(layer_id < GuiLayerIdMax);
    furi_check(furi_mutex_get_owner(instance->access_mutex) == furi_thread_get_current_id());

    lv_display_t* display = instance->display_data[display_id].lv_display;

    switch(layer_id) {
    case GuiLayerIdBottom:
        return lv_display_get_layer_bottom(display);
    case GuiLayerIdActive:
        return lv_display_get_screen_active(display);
    case GuiLayerIdTop:
        return lv_display_get_layer_top(display);
    case GuiLayerIdSystem:
        return lv_display_get_layer_sys(display);
    default:
        furi_crash();
    }
}

