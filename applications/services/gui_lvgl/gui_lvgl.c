#include "gui_lvgl.h"

#include <furi.h>
#include <lvgl.h>

#include <input/input.h>
#include <power_simple/power.h>
#include <storage/storage.h>

#include <lvgl_addons/themes/lv_theme_front.h>

#include <ssd1320/ssd1320.h>
#include <led_display/led_display.h>

#include <lvgl_addons/fs/lv_fs.h>

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

#define TICK_PERIOD_MS (8)

typedef struct {
    GuiInputId id;
    union {
        struct {
            int8_t diff;
            lv_indev_state_t btn_state;
        } encoder;
        struct {
            uint8_t key;
            lv_indev_state_t state;
        } button;
    };
} GuiInputEvent;

typedef struct {
    lv_display_t* lv_display;
    lv_indev_t* lv_indevs[GuiInputIdMax];
    uint8_t* draw_buffer;
    uint8_t* frame_buffer;
} GuiDisplayData;

struct GuiLvgl {
    Power* power;
    Storage* storage;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMutex* access_mutex;
    DotMatrixSrv* dot_matrix;
    GuiDisplayData display_data[GuiDisplayIdMax];
    GuiInputEvent input_event;
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
    UNUSED(area);
    // FURI_LOG_D(
    //     TAG, "Drawing area: (%ld, %ld), (%ld, %ld)", area->x1, area->y1, area->x2, area->y2);

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

static void gui_lvgl_input_read_callback(lv_indev_t* indev, lv_indev_data_t* data) {
    GuiLvgl* instance = lv_indev_get_user_data(indev);

    const GuiInputEvent* event = &instance->input_event;

    if(event->id == GuiInputIdEncoder) {
        furi_assert(lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER);

        if(event->encoder.diff != 0) {
            data->enc_diff = event->encoder.diff;
        } else {
            data->state = event->encoder.btn_state;
        }

    } else if(event->id == GuiInputIdButtons) {
        furi_assert(lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD);

        data->key = event->button.key;
        data->state = event->button.state;

    } else {
        furi_crash("Invalid input id");
    }
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

static bool gui_lvgl_parse_encoder_event(const InputEvent* event, GuiInputEvent* gui_event) {
    bool success = false;

    if(event->key == InputKeyUp || event->key == InputKeyDown) {
        if(event->type == InputTypeShort) {
            gui_event->id = GuiInputIdEncoder;
            gui_event->encoder.diff = (event->key == InputKeyUp) ? 1 : -1;
            gui_event->encoder.btn_state = 0;

            success = true;
        }

    } else if(event->key == InputKeyOk) {
        if(event->type == InputTypePress || event->type == InputTypeRelease) {
            gui_event->id = GuiInputIdEncoder;
            gui_event->encoder.diff = 0;
            gui_event->encoder.btn_state =
                (event->type == InputTypePress) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
            success = true;
        }
    }

    return success;
}

static bool gui_lvgl_parse_buttons_event(const InputEvent* event, GuiInputEvent* gui_event) {
    bool success = false;

    if(event->key == InputKeyBack || event->key == InputKeyStart) {
        if(event->type == InputTypePress || event->type == InputTypeRelease) {
            gui_event->id = GuiInputIdButtons;
            gui_event->button.key = (event->key == InputKeyBack) ? LV_KEY_ESC : LV_KEY_ENTER;
            gui_event->button.state = (event->type == InputTypePress) ? LV_INDEV_STATE_PRESSED :
                                                                        LV_INDEV_STATE_RELEASED;
            success = true;
        }
    }

    return success;
}

static void gui_lvgl_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    GuiLvgl* instance = context;
    const InputEvent* event = message;

    GuiInputEvent gui_event;
    bool event_parsed = gui_lvgl_parse_encoder_event(event, &gui_event) ||
                        gui_lvgl_parse_buttons_event(event, &gui_event);
    if(event_parsed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, &gui_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void gui_lvgl_tick_callback(void* context) {
    furi_assert(context);
    GuiLvgl* instance = context;

    if(furi_mutex_acquire(instance->access_mutex, 0) == FuriStatusOk) {
        lv_timer_periodic_handler();
        furi_mutex_release(instance->access_mutex);
    } else {
        FURI_LOG_T(TAG, "Gui lockup: tick skipped");
    }
}

static void gui_lvgl_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    GuiLvgl* instance = context;
    furi_assert(object == instance->input_queue);

    if(furi_mutex_acquire(instance->access_mutex, 0) == FuriStatusOk) {
        while(furi_message_queue_get(instance->input_queue, &instance->input_event, 0) ==
              FuriStatusOk) {
            const GuiInputId input_id = instance->input_event.id;
            furi_assert(input_id < GuiInputIdMax);

            for(uint32_t i = 0; i < GuiDisplayIdMax; ++i) {
                lv_indev_t* indev = instance->display_data[i].lv_indevs[input_id];

                if(indev != NULL) {
                    lv_indev_read(indev);
                }
            }
        }

        furi_mutex_release(instance->access_mutex);
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

    lv_theme_t* theme = lv_theme_front_init(display_data->lv_display);
    lv_display_set_theme(display_data->lv_display, theme);
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

    lv_theme_t* theme =
        lv_theme_mono_init(display_data->lv_display, true, &lv_font_haxrcorp4089_16);
    lv_display_set_theme(display_data->lv_display, theme);
}

static void gui_lvgl_init_input(GuiLvgl* instance) {
    GuiDisplayData* display_data = &instance->display_data[GuiDisplayIdFront];
    // Created input device gets associated with the default display
    lv_display_t* front = display_data->lv_display;
    lv_display_set_default(front);
    // Newly created LVGL objects will be automatically added to the default group
    lv_group_t* group = lv_group_create();
    lv_group_set_default(group);

    lv_indev_t* encoder = lv_indev_create();
    lv_indev_set_type(encoder, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_mode(encoder, LV_INDEV_MODE_EVENT);
    lv_indev_set_group(encoder, group);
    lv_indev_set_user_data(encoder, instance);
    lv_indev_set_read_cb(encoder, gui_lvgl_input_read_callback);

    display_data->lv_indevs[GuiInputIdEncoder] = encoder;

    lv_indev_t* buttons = lv_indev_create();
    lv_indev_set_type(buttons, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_mode(buttons, LV_INDEV_MODE_EVENT);
    lv_indev_set_group(buttons, group);
    lv_indev_set_user_data(buttons, instance);
    lv_indev_set_read_cb(buttons, gui_lvgl_input_read_callback);

    display_data->lv_indevs[GuiInputIdButtons] = buttons;

    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(input_events, gui_lvgl_input_pubsub_callback, instance);
}

static GuiLvgl* gui_lvgl_alloc(void) {
    GuiLvgl* instance = malloc(sizeof(GuiLvgl));

    // Must be first to ensure that power subsystem is OK
    instance->power = furi_record_open(RECORD_POWER);
    // TODO: Subscribe to power subsystem events
    // TODO: React on overheat or low power budget by limiting brightness

    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->event_loop = furi_event_loop_alloc();
    instance->access_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->dot_matrix = furi_record_open(RECORD_DOT_MATRIX);
    instance->input_queue = furi_message_queue_alloc(16, sizeof(GuiInputEvent));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        gui_lvgl_input_queue_callback,
        instance);

    furi_event_loop_tick_set(
        instance->event_loop, TICK_PERIOD_MS, gui_lvgl_tick_callback, instance);

    lv_init();
    lv_tick_set_cb(furi_get_tick);
    lv_delay_set_cb(furi_delay_ms);
    lv_log_register_print_cb(gui_lvgl_log_callback);

    gui_lvgl_init_front(instance);
    gui_lvgl_init_back(instance);
    gui_lvgl_init_input(instance);
    gui_lvgl_fs_init(instance->storage);

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
