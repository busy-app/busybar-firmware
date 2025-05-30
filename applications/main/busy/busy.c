#include "busy.h"

#include "busy_presets.h"

static void busy_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
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

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_gui_input_callback, instance);

        Widget* root;
        // Create application windows
        root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_window = widget_alloc(root);
        instance->transition_overlay = transition_overlay_alloc(root);
        transition_overlay_set_pressed_widget(
            instance->transition_overlay, instance->front_window);

        root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_window = widget_alloc(root);

        // Create persistent widgets
        instance->timer_card = timer_card_alloc(instance->back_window);
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

    // TODO: Implement audio settings
    audio_set_volume(instance->audio, .5F);
    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdStart);

    return instance;
}

static void busy_free(BusyApp* instance) {
    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    scene_manager_free(instance->scene_manager);
    busy_timer_free(instance->busy_timer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_gui_input_callback);

        transition_overlay_free(instance->transition_overlay);

        widget_free(instance->front_window);
        widget_free(instance->back_window);
    });

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
    furi_event_loop_run(instance->event_loop);
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

    const BusyTransition* transition = &busy_transitions[type];

    with_gui(instance->gui, {
        transition_overlay_set_timings(
            instance->transition_overlay, transition->timings.in_ms, transition->timings.out_ms);
        transition_overlay_set_color(instance->transition_overlay, transition->color);
        transition_overlay_set_color_mode(instance->transition_overlay, transition->color_mode);
        transition_overlay_enable_press_effect(
            instance->transition_overlay, transition->enable_press);

        if(transition->mask_path) {
            transition_overlay_set_mask(instance->transition_overlay, transition->mask_path);
        }

        transition_overlay_set_mask_mode(instance->transition_overlay, transition->mask_mode);
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
