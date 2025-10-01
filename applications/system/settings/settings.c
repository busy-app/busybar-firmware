#include "settings.h"
#include "storage_macros.h"
#include "scenes/settings_scenes.h"

#define SETTINGS_NAV_BAR_HEIGHT 16
#define SETTINGS_MATTER_Q_SIZE  1

static bool settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    SettingsApp* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        settings_send_custom_event(instance, SettingsCustomEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SettingsApp* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SettingsApp* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        if(event == SettingsCustomEventRequiredWifiNotAvailable) {
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdConnectWifi);
        } else {
            scene_manager_handle_custom_event(instance->scene_manager, event);
        }
    }
}

static bool settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;

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

static void settings_handle_matter_event(const void* message, void* context) {
    furi_check(message);
    furi_assert(context);
    const MatterEvent* event = message;
    SettingsApp* app = context;

    SettingsCustomEvent our_event;
    bool do_send_event = false;

    if(event->type == MatterEventTypeCommissioning) {
        MatterCommissioningStatus status = event->commissioning.status;

        static const SettingsCustomEvent event_table[MatterCommissioningStatusMAX] = {
            [MatterCommissioningStatusStarted] = SettingsCustomEventMatterCommStart,
            [MatterCommissioningStatusComplete] = SettingsCustomEventMatterCommComplete,
            [MatterCommissioningStatusFailed] = SettingsCustomEventMatterCommFail,
        };

        our_event = event_table[status];
        do_send_event = true;
    }

    if(do_send_event) settings_send_custom_event(app, our_event);
}

static SettingsApp* settings_alloc(void) {
    SettingsApp* instance = malloc(sizeof(SettingsApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(settings_scenes, COUNT_OF(settings_scenes), instance);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(
            instance->back_nav_bar, SETTINGS_IMG_PATH("settings_back_7x7.bin"));
        nav_bar_set_header_text(instance->back_nav_bar, "SETTINGS");
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
        settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        settings_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdStart);

    instance->matter = furi_record_open(RECORD_MATTER);
    instance->matter_subscription = furi_pubsub_subscribe(
        matter_get_pubsub(instance->matter), settings_handle_matter_event, instance);

    instance->wifi = wifi_poller_alloc();

    return instance;
}

static void settings_free(SettingsApp* instance) {
    wifi_poller_free(instance->wifi);

    furi_pubsub_unsubscribe(matter_get_pubsub(instance->matter), instance->matter_subscription);
    furi_record_close(RECORD_MATTER);

    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t settings_app(void* arg) {
    UNUSED(arg);

    SettingsApp* instance = settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    settings_free(instance);

    return 0;
}

void settings_send_custom_event(SettingsApp* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

void settings_push_location(SettingsApp* instance, const char* location_name) {
    furi_assert(instance);
    furi_assert(location_name);

    with_gui(instance->gui, { nav_bar_push_location(instance->back_nav_bar, location_name); });
}

void settings_pop_location(SettingsApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, { nav_bar_pop_location(instance->back_nav_bar); });
}

bool settings_check_wifi_connectivity(SettingsApp* instance) {
    furi_assert(instance);

    if(wifi_poller_get_state(instance->wifi) & WifiPollerStateLinkUp) {
        return true;
    } else {
        settings_send_custom_event(instance, SettingsCustomEventRequiredWifiNotAvailable);
        return false;
    }
}
