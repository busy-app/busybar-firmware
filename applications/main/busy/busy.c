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

        if(type == BusyApiMessageTypeSetConfig) {
            busy_set_app_config(instance, message.data.set_config.config);

        } else if(type == BusyApiMessageTypeShowTimer) {
            const BusyAppSceneId scene_id =
                scene_manager_get_current_scene_id(instance->scene_manager);

            if(scene_id != BusyAppSceneIdTimer) {
                instance->show_timer_requested = true;
                scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdShowTimer);
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

        if(message.lock) {
            api_lock_unlock(message.lock);
        }
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

static void busy_process_arguments(BusyApp* instance, const char* arg) {
    if(arg == NULL) {
        return;
    }
    if(strstr(arg, BUSY_APP_TIMER_MODE)) {
        instance->run_mode = BusyAppRunModeTimer;
    }
    if(strstr(arg, BUSY_APP_CUSTOM_MODE)) {
        instance->preset_id = BusyAppPresetIdCustom;
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
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->updater = furi_record_open(RECORD_UPDATER);
    instance->loader = furi_record_open(RECORD_LOADER);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->theme = busy_theme_alloc();

    busy_process_arguments(instance, arg);

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
        flex_layout_set_child_widget_grow(
            instance->back_container, nav_bar_get_base(instance->nav_bar), 0);
        nav_bar_set_header_image(
            instance->nav_bar, busy_get_global_preset(instance)->header_img_path);

        instance->timer_card = mirror_card_alloc(back_root);
        mirror_card_set_header_text(instance->timer_card, "ACTIVE");
        mirror_card_set_footer_secondary_text(instance->timer_card, "LEFT");
        widget_set_align(mirror_card_get_base(instance->timer_card), AlignCenter);
        widget_set_margin(mirror_card_get_base(instance->timer_card), 0, 0, 2, 2);
        widget_set_visible(mirror_card_get_base(instance->timer_card), false);

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

    furi_record_create(RECORD_BUSY_APP, instance);

    audio_enable(instance->audio);

    if(instance->run_mode == BusyAppRunModeNormal) {
        scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdStart);
    }

    return instance;
}

static void busy_free(BusyApp* instance) {
    while(!furi_record_destroy(RECORD_BUSY_APP)) {
        // Workaround: wait before all users close the record
        furi_delay_ms(1);
    }

    audio_disable(instance->audio);

    busy_timer_stop(instance->busy_timer);

    busy_set_front_display_blanking(instance, false);

    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_gui_input_callback);

        transition_overlay_free(instance->transition_overlay);

        widget_free(instance->front_window);
        mirror_card_free(instance->timer_card);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_BUSY_TIMER);
    furi_record_close(RECORD_LOADER);
    furi_record_close(RECORD_UPDATER);
    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);

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

void busy_set_priority(BusyApp* instance, bool is_active) {
    furi_assert(instance);
    loader_set_priority(
        instance->loader, is_active ? LOADER_BLOCKING_PRIORITY : LOADER_PASSTHROUGH_PRIORITY);
}

void busy_set_front_display_blanking(BusyApp* instance, bool is_blanked) {
    furi_assert(instance);

    const bool is_show_work_only_enabled = instance->config.is_show_work_only_enabled;
    const bool is_actually_blanked = is_show_work_only_enabled && is_blanked;

    front_display_set_blanked(instance->front_display, is_actually_blanked);
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

bool busy_return_to_start_scene(BusyApp* instance) {
    furi_assert(instance);
    return scene_manager_search_and_switch_to_previous_scene(
        instance->scene_manager, BusyAppSceneIdStart);
}

void busy_exit(BusyApp* instance) {
    furi_assert(instance);
    furi_event_loop_stop(instance->event_loop);
}

void busy_set_app_config(BusyApp* instance, const BusyAppConfig* app_config) {
    furi_assert(instance);

    instance->config = *app_config;

    if(!busy_theme_read(instance->theme, app_config->theme_name)) {
        FURI_LOG_W(TAG, "Setting default theme");
        busy_theme_set_default(instance->theme);
    }

    busy_send_custom_event(instance, BusyCustomEventAppConfigChanged);
}

void busy_get_timer_preset(BusyApp* instance, BusyTimerPreset* timer_config) {
    furi_assert(instance);
    busy_timer_get_preset(instance->busy_timer, busy_get_profile_id(instance), timer_config);
}

void busy_set_timer_preset(BusyApp* instance, BusyTimerPreset* timer_config) {
    furi_assert(instance);
    busy_timer_set_preset(instance->busy_timer, busy_get_profile_id(instance), timer_config);
}

const BusyAppGlobalPreset* busy_get_global_preset(const BusyApp* instance) {
    furi_assert(instance);
    return &busy_app_global_presets[instance->preset_id];
}

BusyTimerProfileId busy_get_profile_id(const BusyApp* instance) {
    furi_assert(instance);
    return busy_get_global_preset(instance)->timer_profile_id;
}
