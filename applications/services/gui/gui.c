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

static void gui_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    Gui* instance = context;

    furi_check(
        furi_message_queue_put(instance->input_queue, message, FuriWaitForever) == FuriStatusOk);
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
        InputEvent event;

        while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
            for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
                GuiDisplay* display = &instance->displays[id];
                if(!WidgetList_empty_p(display->active_widgets)) {
                    Widget* active_widget = *WidgetList_back(display->active_widgets);
                    widget_input(active_widget, &event);
                }
            }
        }

        furi_mutex_release(instance->access_mutex);
    }
}

static void gui_display_remove_active_widget(GuiDisplay* display, Widget* widget) {
    WidgetList_it_t it;
    for(WidgetList_it(it, display->active_widgets); !WidgetList_end_p(it); WidgetList_next(it)) {
        if(*WidgetList_cref(it) == widget) {
            WidgetList_remove(display->active_widgets, it);
            break;
        }
    }
    widget_set_deleted_callback(widget, NULL, NULL);
}

static void gui_display_widget_deleted_callback(Widget* widget, void* context) {
    furi_assert(context);

    GuiDisplay* display = context;
    gui_display_remove_active_widget(display, widget);
}

static void gui_display_add_active_widget(GuiDisplay* display, Widget* widget) {
    gui_display_remove_active_widget(display, widget);
    widget_set_deleted_callback(widget, gui_display_widget_deleted_callback, display);
    WidgetList_push_back(display->active_widgets, widget);
}

static GuiDisplay* gui_find_display_by_widget(Gui* instance, const Widget* widget) {
    lv_display_t* lv_display = lv_obj_get_display((lv_obj_t*)widget);

    for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
        GuiDisplay* display = &instance->displays[id];
        if(display->lv_display == lv_display) {
            return display;
        }
    }

    return NULL;
}

static void gui_init_front(GuiDisplay* display) {
    display->draw_buffer = malloc(FRONT_DRAW_BUFFER_SIZE);
    display->lv_display = lv_display_create(FRONT_W, FRONT_H);
    display->driver = furi_record_open(RECORD_DOT_MATRIX);

    WidgetList_init(display->active_widgets);

    lv_display_set_user_data(display->lv_display, display);
    lv_display_set_flush_cb(display->lv_display, gui_flush_front_callback);
    lv_display_set_color_format(display->lv_display, FRONT_COLOR_FORMAT);
    lv_display_set_buffers(
        display->lv_display,
        display->draw_buffer,
        NULL,
        FRONT_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_theme_t* theme = lv_theme_front_alloc(display->lv_display, &lv_font_tiny5_8);
    lv_display_set_theme(display->lv_display, theme);
}

static void gui_init_back(GuiDisplay* display) {
    ssd1320_init();

    display->draw_buffer = malloc(BACK_DRAW_BUFFER_SIZE);
    display->frame_buffer = malloc(BACK_FRAME_BUFFER_SIZE);
    display->lv_display = lv_display_create(BACK_W, BACK_H);

    WidgetList_init(display->active_widgets);

    lv_display_set_user_data(display->lv_display, display);
    lv_display_set_flush_cb(display->lv_display, gui_flush_back_callback);
    lv_display_set_color_format(display->lv_display, BACK_COLOR_FORMAT);
    lv_display_set_buffers(
        display->lv_display,
        display->draw_buffer,
        NULL,
        BACK_DRAW_BUFFER_SIZE,
        LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_theme_t* theme = lv_theme_front_alloc(display->lv_display, &lv_font_haxrcorp4089_16);
    lv_display_set_theme(display->lv_display, theme);
}

static void gui_init_input(Gui* instance) {
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
    instance->input_queue = furi_message_queue_alloc(16, sizeof(InputEvent));

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

    gui_init_front(&instance->displays[GuiDisplayIdFront]);
    gui_init_back(&instance->displays[GuiDisplayIdBack]);
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

Widget* gui_get_root_widget(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id) {
    furi_check(instance);
    furi_check(display_id < GuiDisplayIdMax);
    furi_check(IS_OWNER(instance->access_mutex));

    lv_display_t* display = instance->displays[display_id].lv_display;
    lv_obj_t* root;

    if(layer_id == GuiLayerIdBottom) {
        root = lv_display_get_layer_bottom(display);
    } else if(layer_id == GuiLayerIdActive) {
        root = lv_display_get_screen_active(display);
    } else if(layer_id == GuiLayerIdTop) {
        root = lv_display_get_layer_top(display);
    } else if(layer_id == GuiLayerIdSystem) {
        root = lv_display_get_layer_sys(display);
    } else {
        furi_crash();
    }

    return (Widget*)root;
}

void gui_add_active_widget(Gui* instance, Widget* widget) {
    furi_check(instance);
    furi_check(widget);
    furi_check(IS_WIDGET_CLASS(widget));
    furi_check(IS_OWNER(instance->access_mutex));

    GuiDisplay* display = gui_find_display_by_widget(instance, widget);
    furi_assert(display);
    gui_display_add_active_widget(display, widget);
}

void gui_remove_active_widget(Gui* instance, Widget* widget) {
    furi_check(instance);
    furi_check(widget);
    furi_check(IS_WIDGET_CLASS(widget));
    furi_check(IS_OWNER(instance->access_mutex));

    GuiDisplay* display = gui_find_display_by_widget(instance, widget);
    furi_assert(display);
    gui_display_remove_active_widget(display, widget);
}
