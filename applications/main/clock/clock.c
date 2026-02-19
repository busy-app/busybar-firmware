#include "clock_i.h"

#include <settings_helpers/gui_params.h>

#include <furi.h>

#define TAG "clock"

#define INPUT_QUEUE_CAPACITY   8
#define INPUT_QUEUE_TIMEOUT_MS 3000

#define EVENT_QUEUE_CAPACITY   8
#define EVENT_QUEUE_TIMEOUT_MS 3000

#define TIMER_INTERVAL_MS 100

static bool thread_signal_callback(uint32_t signal, void* argument, void* context) {
    UNUSED(argument);

    furi_assert(context);

    ThisInstance* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    default:
        return false;
    }
}

static void input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    ThisInstance* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            if(!scene_manager_handle_back_event(instance->scene_manager)) {
                desktop_replace_current_app(instance->desktop, "apps_menu", THIS_APP_NAME);
            }
        }
    }
}

static void event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    ThisInstance* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ThisInstance* instance = context;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        FuriStatus queue_status =
            furi_message_queue_put(instance->input_queue, event, INPUT_QUEUE_TIMEOUT_MS);

        if(queue_status != FuriStatusOk) FURI_LOG_E(TAG, "Input queue failure");

        return true;
    }

    return false;
}

static void clock_timer_callback(void* context) {
    ThisInstance* instance = context;

    clock_app_fire_event(instance, ThisEventTimerUpdate);
}

static ThisInstance* this_alloc(void) {
    ThisInstance* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_CAPACITY, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_CAPACITY, sizeof(uint32_t));
    instance->scene_manager = scene_manager_alloc(clock_app_scenes, ThisSceneIdxsCount, instance);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->sntp = furi_record_open(RECORD_SNTP);
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->updater = furi_record_open(RECORD_UPDATER);

    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, clock_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    updater_pause_autoupdates(instance->updater);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(
            instance->back_nav_bar, SHARED_IMG_PATH("apps_menu_back_12x12.bin"));
        nav_bar_push_location(instance->back_nav_bar, "CLOCK");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), 14);
        widget_set_margin(nav_bar_get_base(instance->back_nav_bar), 1, 0, 0, 2);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        event_queue_callback,
        instance);

    furi_event_loop_timer_start(instance->timer, furi_ms_to_ticks(TIMER_INTERVAL_MS));

    scene_manager_next_scene(instance->scene_manager, ThisSceneIdxMain);

    return instance;
}

static void this_free(ThisInstance* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    updater_resume_autoupdates(instance->updater);

    furi_record_close(RECORD_UPDATER);
    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_SNTP);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);

    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t clock_app_entry(void* argument) {
    UNUSED(argument);

    ThisInstance* instance = this_alloc();

    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);

    furi_thread_set_signal_callback(thread, NULL, NULL);

    this_free(instance);

    return 0;
}

void clock_app_fire_event(ThisInstance* instance, uint32_t event) {
    furi_assert(instance);

    FuriStatus queue_status =
        furi_message_queue_put(instance->event_queue, &event, EVENT_QUEUE_TIMEOUT_MS);

    if(queue_status != FuriStatusOk) FURI_LOG_E(TAG, "Event queue failure");
}
