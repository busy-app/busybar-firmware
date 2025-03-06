#include "gui_i.h"

#include <lvgl_addons/fs/lv_fs.h>
#include <lvgl_addons/themes/lv_theme_front.h>

#define TAG "Gui"

#define IS_OWNER(mtx) (furi_mutex_get_owner(mtx) == furi_thread_get_current_id())

static void
    gui_flush_front_callback(lv_display_t* lv_display, const lv_area_t* area, uint8_t* px_map) {
    UNUSED(area);

    GuiDisplay* display = lv_display_get_user_data(lv_display);
    furi_check(px_map == display->draw_buffer);

    dot_matrix_draw(display->driver, display->draw_buffer);

    lv_display_flush_ready(lv_display);
}

// TODO: Optimise conversion?
static void gui_l8_to_l4(uint8_t* dst, const uint8_t* src) {
    for(uint32_t i = 0; i < BACK_FRAME_BUFFER_SIZE; ++i) {
        const uint32_t draw_idx = 2 * i;
        dst[i] = (src[draw_idx] >> 4) | (src[draw_idx + 1] & 0xF0);
    }
}

static void
    gui_flush_back_callback(lv_display_t* lv_display, const lv_area_t* area, uint8_t* px_map) {
    UNUSED(area);

    GuiDisplay* display = lv_display_get_user_data(lv_display);
    furi_check(px_map == display->draw_buffer);

    gui_l8_to_l4(display->frame_buffer, display->draw_buffer);
    ssd1320_draw(display->frame_buffer);

    lv_display_flush_ready(lv_display);
}

static void gui_input_read_callback(lv_indev_t* indev, lv_indev_data_t* data) {
    const GuiInputEvent* event = lv_indev_get_user_data(indev);

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

static void gui_log_callback(lv_log_level_t level, const char* buf) {
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

static bool gui_parse_encoder_event(const InputEvent* event, GuiInputEvent* gui_event) {
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

static bool gui_parse_buttons_event(const InputEvent* event, GuiInputEvent* gui_event) {
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

static void gui_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    Gui* instance = context;
    const InputEvent* event = message;

    GuiInputEvent gui_event;
    bool event_parsed = gui_parse_encoder_event(event, &gui_event) ||
                        gui_parse_buttons_event(event, &gui_event);
    if(event_parsed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, &gui_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void gui_tick_callback(void* context) {
    furi_assert(context);
    Gui* instance = context;

    if(furi_mutex_acquire(instance->access_mutex, 0) == FuriStatusOk) {
        lv_timer_periodic_handler();
        furi_mutex_release(instance->access_mutex);
    } else {
        FURI_LOG_T(TAG, "Gui lockup: tick skipped");
    }
}

static void gui_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Gui* instance = context;
    furi_assert(object == instance->input_queue);

    if(furi_mutex_acquire(instance->access_mutex, 0) == FuriStatusOk) {
        while(furi_message_queue_get(instance->input_queue, &instance->input_event, 0) ==
              FuriStatusOk) {
            const GuiInputId input_id = instance->input_event.id;
            furi_assert(input_id < GuiInputIdMax);

            for(uint32_t i = 0; i < GuiDisplayIdMax; ++i) {
                GuiDisplay* display = &instance->displays[i];
                lv_indev_t* indev = display->lv_indevs[input_id];

                if(indev != NULL) {
                    lv_indev_read(indev);
                }
            }
        }

        furi_mutex_release(instance->access_mutex);
    }
}

static void gui_init_front(Gui* instance) {
    GuiDisplay* display = &instance->displays[GuiDisplayIdFront];

    display->draw_buffer = malloc(FRONT_DRAW_BUFFER_SIZE);
    display->lv_display = lv_display_create(FRONT_W, FRONT_H);
    display->driver = furi_record_open(RECORD_DOT_MATRIX);

    lv_display_set_user_data(display->lv_display, display);
    lv_display_set_flush_cb(display->lv_display, gui_flush_front_callback);
    lv_display_set_color_format(display->lv_display, FRONT_COLOR_FORMAT);
    lv_display_set_buffers(
        display->lv_display,
        display->draw_buffer,
        NULL,
        FRONT_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_theme_t* theme = lv_theme_front_init(display->lv_display);
    lv_display_set_theme(display->lv_display, theme);
}

static void gui_init_back(Gui* instance) {
    ssd1320_init();

    GuiDisplay* display = &instance->displays[GuiDisplayIdBack];

    display->draw_buffer = malloc(BACK_DRAW_BUFFER_SIZE);
    display->frame_buffer = malloc(BACK_FRAME_BUFFER_SIZE);
    display->lv_display = lv_display_create(BACK_W, BACK_H);

    lv_display_set_user_data(display->lv_display, display);
    lv_display_set_flush_cb(display->lv_display, gui_flush_back_callback);
    lv_display_set_color_format(display->lv_display, BACK_COLOR_FORMAT);
    lv_display_set_buffers(
        display->lv_display,
        display->draw_buffer,
        NULL,
        BACK_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_theme_t* theme = lv_theme_mono_init(display->lv_display, true, &lv_font_haxrcorp4089_16);
    lv_display_set_theme(display->lv_display, theme);
}

static void gui_init_display_input(GuiDisplay* display, GuiInputEvent* event) {
    // Created input device gets associated with the default display
    lv_display_set_default(display->lv_display);
    // Create and initialise encoder
    lv_indev_t* encoder = lv_indev_create();
    lv_indev_set_type(encoder, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_mode(encoder, LV_INDEV_MODE_EVENT);
    lv_indev_set_user_data(encoder, event);
    lv_indev_set_read_cb(encoder, gui_input_read_callback);
    display->lv_indevs[GuiInputIdEncoder] = encoder;
    // Create and initialise buttons
    lv_indev_t* buttons = lv_indev_create();
    lv_indev_set_type(buttons, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_mode(buttons, LV_INDEV_MODE_EVENT);
    lv_indev_set_user_data(buttons, event);
    lv_indev_set_read_cb(buttons, gui_input_read_callback);
    display->lv_indevs[GuiInputIdButtons] = buttons;
}

static void gui_init_input(Gui* instance) {
    for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
        gui_init_display_input(&instance->displays[id], &instance->input_event);
    }

    // TODO: Group management will be done in the Screen class
    lv_group_t* default_group = lv_group_create();
    lv_group_set_default(default_group);

    GuiDisplay* front = &instance->displays[GuiDisplayIdFront];
    for(GuiInputId id = 0; id < GuiInputIdMax; ++id) {
        lv_indev_set_group(front->lv_indevs[id], default_group);
    }

    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(input_events, gui_input_pubsub_callback, instance);
}

static Gui* gui_alloc(void) {
    Gui* instance = malloc(sizeof(Gui));
    // Must be first to ensure that power subsystem is OK
    instance->power = furi_record_open(RECORD_POWER);
    // TODO: Subscribe to power subsystem events
    // TODO: React on overheat or low power budget by limiting brightness
    instance->event_loop = furi_event_loop_alloc();
    instance->access_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->dot_matrix = furi_record_open(RECORD_DOT_MATRIX);
    instance->input_queue = furi_message_queue_alloc(16, sizeof(GuiInputEvent));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        gui_input_queue_callback,
        instance);

    furi_event_loop_tick_set(instance->event_loop, TICK_PERIOD_MS, gui_tick_callback, instance);

    lv_init();
    lv_storage_driver_init();
    lv_tick_set_cb(furi_get_tick);
    lv_delay_set_cb(furi_delay_ms);
    lv_log_register_print_cb(gui_log_callback);

    gui_init_front(instance);
    gui_init_back(instance);
    gui_init_input(instance);

    furi_record_create(RECORD_GUI, instance);
    return instance;
}

int gui_srv(void* arg) {
    UNUSED(arg);

    Gui* instance = gui_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

// Public API functions

void gui_lock(Gui* instance) {
    furi_check(instance);
    furi_check(furi_mutex_acquire(instance->access_mutex, FuriWaitForever) == FuriStatusOk);
}

void gui_unlock(Gui* instance) {
    furi_check(instance);
    furi_check(furi_mutex_release(instance->access_mutex) == FuriStatusOk);
}

lv_obj_t* gui_get_layer(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id) {
    furi_check(instance);
    furi_check(display_id < GuiDisplayIdMax);
    furi_check(layer_id < GuiLayerIdMax);
    furi_check(IS_OWNER(instance->access_mutex));

    lv_display_t* display = instance->displays[display_id].lv_display;

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
