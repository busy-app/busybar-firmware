#include "busy_i.h"
#include "busy_presets.h"

#define BUSY_NAV_BAR_HEIGHT 20

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

static void busy_api_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    furi_assert(instance->api_queue == object);

    BusyApiMessage message;
    while(furi_message_queue_get(instance->api_queue, &message, 0) == FuriStatusOk) {
        const BusyApiMessageType type = message.type;

        if(type == BusyApiMessageTypeShowTimer) {
            const BusyAppSceneId scene_id =
                scene_manager_get_current_scene_id(instance->scene_manager);

            if(scene_id != BusyAppSceneIdTimer) {
                busy_go_to_show_timer_scene(instance);
            }

        } else if(type == BusyApiMessageTypeRequestExit) {
            if(instance->run_mode == BusyAppRunModeTimer) {
                // App was launched by the timer, exit
                busy_exit(instance);
            } else {
                // App was launched normally, return to start menu
                busy_send_custom_event(instance, BusyCustomEventReturnToStart);
            }

        } else {
            furi_crash("Invalid BusyApiMessageType value");
        }

        api_lock_unlock(message.lock);
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

static void busy_set_default_settings(BusySettings* settings) {
    strcpy(settings->theme_name, "default");
}

static void busy_load_settings(BusyApp* instance) {
    BusySettings* settings = &instance->settings;

    if(!busy_settings_load(settings)) {
        FURI_LOG_W(TAG, "Loading default settings");
        busy_set_default_settings(settings);
        busy_settings_save(settings);
    }

    if(!busy_theme_read(instance->theme, settings->theme_name)) {
        FURI_LOG_W(TAG, "Setting default theme");
        busy_theme_set_default(instance->theme);
    }
}

static BusyApp* busy_alloc(const char* arg) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->api_queue = furi_message_queue_alloc(1, sizeof(BusyApiMessage));
    instance->scene_manager = scene_manager_alloc(busy_scenes, BusyAppSceneIdMax, instance);
    instance->busy_timer = furi_record_open(RECORD_BUSY_TIMER);
    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->matter = furi_record_open(RECORD_MATTER);
    instance->theme = busy_theme_alloc();

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_set_matter(instance, false);

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

        instance->timer_card = timer_card_alloc(back_root);
        widget_set_pos_y(timer_card_get_base(instance->timer_card), 2);

        // Create application window on Back display
        instance->back_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(instance->back_container, instance->back_window, 1);
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

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        busy_api_queue_callback,
        instance);

    busy_load_settings(instance);

    if(arg && strcmp(arg, BUSY_APP_TIMER_MODE) == 0) {
        instance->run_mode = BusyAppRunModeTimer;
        busy_go_to_show_timer_scene(instance);

    } else {
        instance->run_mode = BusyAppRunModeNormal;
        scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdStart);
    }

    furi_record_create(RECORD_BUSY_APP, instance);
    return instance;
}

static void busy_free(BusyApp* instance) {
    while(!furi_record_destroy(RECORD_BUSY_APP)) {
        // Workaround: wait before all users close the record
        furi_delay_ms(1);
    }

    if(busy_timer_get_state(instance->busy_timer) != BusyTimerStateIdle) {
        busy_timer_stop(instance->busy_timer);
    }

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_set_matter(instance, false);

    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_gui_input_callback);

        transition_overlay_free(instance->transition_overlay);

        widget_free(instance->front_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_BUSY_TIMER);
    furi_record_close(RECORD_MATTER);
    furi_record_close(RECORD_STATUS_LIGHTS);
    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->api_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_message_queue_free(instance->api_queue);
    furi_event_loop_free(instance->event_loop);
    busy_theme_free(instance->theme);

    free(instance);
}

int32_t busy_app(void* arg) {
    BusyApp* instance = busy_alloc(arg);
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

    const BusyStatusLightsPreset* preset = &busy_status_lights[type];
    status_lights_run_preset(instance->status_lights, preset->preset, preset->color);
}

void busy_set_matter(BusyApp* instance, bool switch_state) {
    furi_assert(instance);
    matter_set_switch_state(instance->matter, switch_state);
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

void busy_go_to_show_timer_scene(BusyApp* instance) {
    furi_assert(instance);

    instance->show_timer_requested = true;
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdShowTimer);
}

bool busy_return_to_start_scene(BusyApp* instance) {
    furi_assert(instance);
    return scene_manager_search_and_switch_to_previous_scene(
        instance->scene_manager, BusyAppSceneIdStart);
}

void busy_exit(BusyApp* instance) {
    furi_assert(instance);
    furi_event_loop_stop(instance->event_loop);
}
