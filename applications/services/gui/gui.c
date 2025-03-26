#include "gui_i.h"

#include <lvgl_addons/fs/lv_fs.h>
#include <lvgl_addons/themes/lv_theme_front.h>

#define TAG "Gui"

static void
    gui_flush_front_callback(lv_display_t* lv_display, const lv_area_t* area, uint8_t* px_map) {
    UNUSED(area);

    GuiDisplay* display = lv_display_get_user_data(lv_display);
    furi_check(px_map == display->draw_buffer);

    dot_matrix_draw(display->driver, display->draw_buffer);

    lv_display_flush_ready(lv_display);
}

static void
    gui_flush_back_callback(lv_display_t* lv_display, const lv_area_t* area, uint8_t* px_map) {
    UNUSED(area);

    GuiDisplay* display = lv_display_get_user_data(lv_display);
    furi_check(px_map == display->draw_buffer);

    back_display_draw(display->driver, display->draw_buffer);

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

static lv_obj_t* gui_get_layer_root(Gui* instance, GuiDisplayId display_id, GuiLayerId layer_id) {
    lv_display_t* display = instance->displays[display_id].lv_display;
    lv_obj_t* layer;

    if(layer_id == GuiLayerIdBottom) {
        layer = lv_display_get_layer_bottom(display);
    } else if(layer_id == GuiLayerIdMain) {
        layer = lv_display_get_screen_active(display);
    } else if(layer_id == GuiLayerIdTop) {
        layer = lv_display_get_layer_top(display);
    } else if(layer_id == GuiLayerIdSystem) {
        layer = lv_display_get_layer_sys(display);
    } else {
        furi_crash();
    }

    return layer;
}

static bool gui_layer_feed_input(GuiLayer* layer, const InputEvent* event) {
    bool consumed = false;

    for(GuiDisplayId display_id = 0; display_id < GuiDisplayIdMax; ++display_id) {
        lv_obj_t* root = layer->root_objs[display_id];
        const uint32_t child_count = lv_obj_get_child_count(root);

        for(uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* child = lv_obj_get_child(root, i);

            if(IS_WIDGET_CLASS(child)) {
                if(widget_input((Widget*)child, event)) {
                    consumed = true;
                }
            }
        }
    }

    return consumed;
}

static bool gui_layer_feed_user_input(GuiLayer* layer, const InputEvent* event) {
    bool consumed = false;

    GuiInputItemList_it_t it;
    for(GuiInputItemList_it(it, layer->input_list); !GuiInputItemList_end_p(it);
        GuiInputItemList_next(it)) {
        const GuiInputItem* item = GuiInputItemList_cref(it);
        if(item->callback(event, item->context)) {
            consumed = true;
        }
    }

    return consumed;
}

static void gui_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Gui* instance = context;
    furi_assert(object == instance->input_queue);

    if(furi_mutex_acquire(instance->access_mutex, 0) == FuriStatusOk) {
        InputEvent event;

        while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
            for(GuiLayerId id = GuiLayerIdSystem; id < GuiLayerIdMax; ++id) {
                GuiLayer* layer = &instance->layers[id];
                if(gui_layer_feed_input(layer, &event)) {
                    break;
                }
                if(gui_layer_feed_user_input(layer, &event)) {
                    break;
                }
            }
        }

        furi_mutex_release(instance->access_mutex);
    }
}

static void gui_init_front(GuiDisplay* display) {
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

    lv_theme_t* theme = lv_theme_front_alloc(display->lv_display, &lv_font_tiny5_8);
    lv_display_set_theme(display->lv_display, theme);
}

static void gui_init_back(GuiDisplay* display) {
    display->draw_buffer = malloc(BACK_DRAW_BUFFER_SIZE);
    display->lv_display = lv_display_create(BACK_W, BACK_H);
    display->driver = furi_record_open(RECORD_BACK_DISPLAY);

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

static void gui_init_layers(Gui* instance) {
    for(GuiLayerId layer_id = GuiLayerIdSystem; layer_id < GuiLayerIdMax; ++layer_id) {
        GuiLayer* layer = &instance->layers[layer_id];
        for(GuiDisplayId display_id = 0; display_id < GuiDisplayIdMax; ++display_id) {
            layer->root_objs[display_id] = gui_get_layer_root(instance, display_id, layer_id);
        }
        GuiInputItemList_init(layer->input_list);
    }
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
    gui_init_layers(instance);
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

GuiLayer* gui_get_layer(Gui* instance, GuiLayerId layer_id) {
    furi_check(instance);
    furi_check(layer_id < GuiLayerIdMax);

    return &instance->layers[layer_id];
}

Widget* gui_layer_get_root_widget(GuiLayer* layer, GuiDisplayId display_id) {
    furi_check(layer);
    furi_check(display_id < GuiDisplayIdMax);

    return (Widget*)layer->root_objs[display_id];
}

void gui_layer_add_input_callback(GuiLayer* layer, GuiInputCallback callback, void* context) {
    furi_check(layer);
    furi_check(callback);

    GuiInputItemList_it_t it;
    for(GuiInputItemList_it(it, layer->input_list); !GuiInputItemList_end_p(it);
        GuiInputItemList_next(it)) {
        const GuiInputItem* item = GuiInputItemList_cref(it);
        if(item->callback == callback) {
            furi_crash(TAG ": Callback alrady registered");
        }
    }

    GuiInputItem* item = GuiInputItemList_push_new(layer->input_list);
    item->callback = callback;
    item->context = context;
}

void gui_layer_remove_input_callback(GuiLayer* layer, GuiInputCallback callback) {
    furi_check(layer);
    furi_check(callback);

    GuiInputItemList_it_t it;
    for(GuiInputItemList_it(it, layer->input_list); !GuiInputItemList_end_p(it);
        GuiInputItemList_next(it)) {
        const GuiInputItem* item = GuiInputItemList_cref(it);
        if(item->callback == callback) {
            GuiInputItemList_remove(layer->input_list, it);
            break;
        }
    }
}
