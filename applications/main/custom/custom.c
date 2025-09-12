#include "custom.h"
#include "custom_presets.h"

#define CUSTOM_NAV_BAR_HEIGHT 20

static bool custom_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    CustomApp* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        custom_send_custom_event(instance, CustomCustomEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void custom_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    furi_assert(instance->input_queue == object);

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                if(!scene_manager_handle_back_event(instance->scene_manager)) {
                    furi_event_loop_stop(instance->event_loop);
                    break;
                }
            }
        }
    }
}

static void custom_event_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    furi_assert(instance->event_queue == object);

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool custom_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    CustomApp* instance = context;

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

static CustomApp* custom_alloc(void) {
    CustomApp* instance = malloc(sizeof(CustomApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager = scene_manager_alloc(custom_scenes, CustomAppSceneIdMax, instance);
    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->desktop = furi_record_open(RECORD_DESKTOP);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, custom_gui_input_callback, instance);

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
        widget_set_height(nav_bar_get_base(instance->nav_bar), CUSTOM_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->nav_bar), 2, 2, 0, 0);
        nav_bar_set_header_image(instance->nav_bar, CUSTOM_IMG_PATH("header_custom_42x16.bin"));
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
        custom_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        custom_event_queue_callback,
        instance);

    custom_set_status_lights(instance, CustomStatusLightsTypeOff);

    scene_manager_next_scene(instance->scene_manager, CustomAppSceneIdStart);

    return instance;
}

static void custom_free(CustomApp* instance) {
    custom_set_status_lights(instance, CustomStatusLightsTypeOff);

    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, custom_gui_input_callback);

        transition_overlay_free(instance->transition_overlay);

        widget_free(instance->front_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_DESKTOP);
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

int32_t custom_app(void* arg) {
    UNUSED(arg);

    CustomApp* instance = custom_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, custom_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    custom_free(instance);

    return 0;
}

void custom_send_custom_event(CustomApp* instance, uint32_t custom_event) {
    furi_assert(instance);
    furi_check(
        furi_message_queue_put(instance->event_queue, &custom_event, FuriWaitForever) ==
        FuriStatusOk);
}

void custom_prepare_transition(CustomApp* instance, CustomTransitionType type) {
    furi_assert(instance);
    furi_assert(type < CustomTransitionTypeMax);

    with_gui(instance->gui, {
        transition_overlay_set_preset(instance->transition_overlay, &custom_transitions[type]);
        transition_overlay_show(instance->transition_overlay);
    });
}

void custom_start_transition(CustomApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, { transition_overlay_start(instance->transition_overlay); });
}

void custom_set_status_lights(CustomApp* instance, CustomStatusLightsType type) {
    furi_assert(instance);
    furi_assert(type < CustomStatusLightsTypeMax);

    const CustomStatusLightsPreset* preset = &custom_status_lights[type];
    status_lights_run_preset(instance->status_lights, preset->preset, preset->color);
}

void custom_push_location(CustomApp* instance, const char* location_name) {
    furi_assert(instance);
    furi_assert(location_name);

    with_gui(instance->gui, { nav_bar_push_location(instance->nav_bar, location_name); });
}

void custom_pop_location(CustomApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, { nav_bar_pop_location(instance->nav_bar); });
}
