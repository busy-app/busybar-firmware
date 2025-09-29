#include "busy.h"
#include "busy_presets.h"

#define BUSY_NAV_BAR_HEIGHT 20

static bool busy_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    BusyApp* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        busy_send_custom_event(instance, BusyCustomEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void busy_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    furi_assert(instance->input_queue == object);

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void busy_event_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    furi_assert(instance->event_queue == object);

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool busy_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    BusyApp* instance = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            consumed = true;
        }
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static BusyApp* busy_alloc(void) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager = scene_manager_alloc(busy_scenes, BusyAppSceneIdMax, instance);
    instance->busy_timer = busy_timer_alloc();
    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->matter = furi_record_open(RECORD_MATTER);

    if(!busy_settings_load(&instance->settings)) {
        FURI_LOG_W(TAG, "Loading default settings");
        // Get default timer config
        busy_timer_get_config(instance->busy_timer, &instance->settings.timer_config);
        busy_settings_save(&instance->settings);

    } else {
        busy_timer_set_config(instance->busy_timer, &instance->settings.timer_config);
    }

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_gui_input_callback, instance);

        // Create application window on Front display
        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_window = widget_alloc(front_root);

        // Create persistent widgets on Front display
        instance->transition_overlay = transition_overlay_alloc(front_root);
        transition_overlay_set_pressed_widget(
            instance->transition_overlay, instance->front_window);

        // Create container on Back display
        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);
        flex_layout_set_spacing(instance->back_container, 2);

        // Create persistent widgets on Back display
        instance->nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        widget_set_height(nav_bar_get_base(instance->nav_bar), BUSY_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->nav_bar), 2, 2, 0, 0);
        nav_bar_set_header_image(instance->nav_bar, BUSY_IMG_PATH("header_busy_39x16.bin"));
        flex_layout_set_child_widget_grow(
            instance->back_container, nav_bar_get_base(instance->nav_bar), 0);

        // Create application window on Back display
        instance->back_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(instance->back_container, instance->back_window, 1);

        instance->timer_card = timer_card_alloc(flex_layout_get_base(instance->back_container));
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        busy_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        busy_event_queue_callback,
        instance);

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_set_matter(instance, false);

    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdStart);

    return instance;
}

static void busy_free(BusyApp* instance) {
    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_set_matter(instance, false);

    scene_manager_free(instance->scene_manager);
    busy_timer_free(instance->busy_timer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_gui_input_callback);

        transition_overlay_free(instance->transition_overlay);

        widget_free(instance->front_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_MATTER);
    furi_record_close(RECORD_STATUS_LIGHTS);
    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t busy_app(void* arg) {
    UNUSED(arg);

    BusyApp* instance = busy_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, busy_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    busy_free(instance);

    return 0;
}

void busy_send_custom_event(BusyApp* instance, uint32_t custom_event) {
    furi_assert(instance);
    furi_check(
        furi_message_queue_put(instance->event_queue, &custom_event, FuriWaitForever) ==
        FuriStatusOk);
}

void busy_prepare_transition(BusyApp* instance, BusyTransitionType type) {
    furi_assert(instance);
    furi_assert(type < BusyTransitionTypeMax);

    with_gui(instance->gui, {
        transition_overlay_set_preset(instance->transition_overlay, &busy_transitions[type]);
        transition_overlay_show(instance->transition_overlay);
    });
}

void busy_start_transition(BusyApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, { transition_overlay_start(instance->transition_overlay); });
}

void busy_set_status_lights(BusyApp* instance, BusyStatusLightsType type) {
    furi_assert(instance);
    furi_assert(type < BusyStatusLightsTypeMax);

    status_lights_send_command(instance->status_lights, &busy_status_lights[type]);
}

void busy_set_matter(BusyApp* instance, bool switch_state) {
    furi_assert(instance);
    MatterVirtualDeviceState device_state = {
        .device = MatterVirtualDeviceSwitch1,
        .bool_val = switch_state,
    };
    matter_set_state(instance->matter, device_state);
}

void busy_push_location(BusyApp* instance, const char* location_name) {
    furi_assert(instance);
    furi_assert(location_name);

    with_gui(instance->gui, { nav_bar_push_location(instance->nav_bar, location_name); });
}

void busy_pop_location(BusyApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, { nav_bar_pop_location(instance->nav_bar); });
}
