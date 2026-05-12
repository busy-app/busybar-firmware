#include "matter_settings_i.h"
#include "scenes/matter_scenes.h"
#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

// Time for the user to understand the status being shown to them
#define USER_REACTION_TIME_MS 500

void matter_settings_acknowledge_status(MatterStatusAck* status_ack) {
    furi_assert(status_ack);
    status_ack->user_knowledge_timestamp = furi_hal_rtc_get_timestamp_ms() - USER_REACTION_TIME_MS;
}

static bool matter_settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    MatterSettings* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        matter_settings_send_custom_event(instance, AppEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void matter_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    MatterSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void matter_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    MatterSettings* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        if(event >= AppEventMatterCommStart && event <= AppEventMatterCommFail) {
            static const SceneId event_to_scene[] = {
                [AppEventMatterCommStart] = SceneIdCommissionStart,
                [AppEventMatterCommComplete] = SceneIdCommissionDone,
                [AppEventMatterCommFail] = SceneIdCommissionFail,
            };
            scene_manager_replace_current_scene(instance->scene_manager, event_to_scene[event]);

        } else if(event == AppEventRequiredWifiNotAvailable) {
            desktop_replace_current_app(instance->desktop, WIFI_SETTINGS_APP, NULL);

        } else {
            scene_manager_handle_custom_event(instance->scene_manager, event);
        }
    }
}

static bool matter_settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    MatterSettings* instance = context;

    bool consumed = false;
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

static void matter_settings_handle_matter_event(const void* message, void* context) {
    furi_check(message);
    furi_assert(context);
    const MatterEvent* event = message;
    MatterSettings* app = context;

    AppEvent our_event;
    bool do_send_event = false;

    if(event->type == MatterEventTypeCommissioning) {
        MatterCommissioningStatus status = event->commissioning.status;

        static const AppEvent event_table[MatterCommissioningStatusMAX] = {
            [MatterCommissioningStatusStarted] = AppEventMatterCommStart,
            [MatterCommissioningStatusComplete] = AppEventMatterCommComplete,
            [MatterCommissioningStatusFailed] = AppEventMatterCommFail,
        };

        our_event = event_table[status];
        do_send_event = true;
    }

    if(do_send_event) matter_settings_send_custom_event(app, our_event);
}

static void matter_settings_set_initial_scenes(MatterSettings* app) {
    furi_assert(app);

    MatterCommissionedFabrics fabrics;
    if(matter_get_commissioned_fabrics(app->matter, &fabrics) != MatterStatusOk) {
        scene_manager_next_scene(app->scene_manager, SceneIdWrecked);
        return;
    }

    scene_manager_next_scene(app->scene_manager, SceneIdMain);

    if(fabrics.last_status == MatterCommissioningStatusStarted) {
        scene_manager_next_scene(app->scene_manager, SceneIdCommissionStart);
    }

    bool user_seen_current_status = app->status_ack->user_knowledge_timestamp >=
                                    fabrics.last_status_at;

    if(!user_seen_current_status) {
        if(fabrics.last_status == MatterCommissioningStatusComplete) {
            scene_manager_next_scene(app->scene_manager, SceneIdCommissionDone);
        } else if(fabrics.last_status == MatterCommissioningStatusFailed) {
            scene_manager_next_scene(app->scene_manager, SceneIdCommissionFail);
        }
    }
}

static MatterSettings* matter_settings_alloc(void) {
    MatterSettings* instance = malloc(sizeof(MatterSettings));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(matter_scenes, COUNT_OF(matter_scenes), instance);

    instance->matter = furi_record_open(RECORD_MATTER);
    instance->matter_subscription = furi_pubsub_subscribe(
        matter_get_pubsub(instance->matter), matter_settings_handle_matter_event, instance);

    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);

    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    instance->brightness_control = furi_record_open(RECORD_BRIGHTNESS_CONTROL);

    instance->wifi = furi_record_open(RECORD_WIFI);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, matter_settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_set_header_text(instance->back_nav_bar, "SETTINGS");
        nav_bar_push_location(instance->back_nav_bar, "SMART HOME");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 2, 2, 0, 0);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        matter_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        matter_settings_event_queue_callback,
        instance);

    if(!furi_record_exists(RECORD_MATTER_SETTINGS_STATUS_ACK)) {
        MatterStatusAck* status_ack = malloc(sizeof(MatterStatusAck));
        furi_record_create(RECORD_MATTER_SETTINGS_STATUS_ACK, status_ack);
        instance->status_ack = status_ack;
    } else {
        instance->status_ack = furi_record_open(RECORD_MATTER_SETTINGS_STATUS_ACK);
    }

    matter_settings_set_initial_scenes(instance);

    return instance;
}

static void matter_settings_free(MatterSettings* instance) {
    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, matter_settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_pubsub_unsubscribe(matter_get_pubsub(instance->matter), instance->matter_subscription);
    furi_record_close(RECORD_MATTER);

    furi_record_close(RECORD_WIFI);
    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_STATUS_LIGHTS);
    furi_record_close(RECORD_BRIGHTNESS_CONTROL);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    scene_manager_free(instance->scene_manager);
    furi_message_queue_free(instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t matter_settings_entry(void* arg) {
    if(arg) {
        SettingsAppDescriptor* descriptor = arg;

        furi_string_set_str(descriptor->front_title, "Smart home");
        furi_string_set_str(descriptor->back_title, "Smart home");
        furi_string_set_str(descriptor->front_icon, IMG_PATH("house_front_8x8.image"));
        furi_string_set_str(descriptor->back_icon, IMG_PATH("house_back_11x11.image"));

        return 0;
    }

    MatterSettings* instance = matter_settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, matter_settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    matter_settings_free(instance);

    return 0;
}

void matter_settings_send_custom_event(MatterSettings* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

bool matter_settings_check_wifi_connectivity(MatterSettings* instance) {
    furi_assert(instance);

    bool is_connected = false;
    do {
        WifiInfo info;
        if(wifi_get_info(instance->wifi, &info) != WifiStatusOk) break;

        is_connected = info.state == WifiStateConnected;
    } while(0);

    if(!is_connected) {
        matter_settings_send_custom_event(instance, AppEventRequiredWifiNotAvailable);
    }

    return is_connected;
}

bool matter_settings_exit_if_last(MatterSettings* instance) {
    furi_assert(instance);
    if(!scene_manager_has_previous_scene(instance->scene_manager, SceneIdMain)) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        return true;
    }
    return false;
}
