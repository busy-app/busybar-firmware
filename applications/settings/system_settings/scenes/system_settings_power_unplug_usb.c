
#include "../system_settings.h"
#include <settings_helpers/status_view.h>
#include "../settings_helpers/gui_params.h"

#include <power/power_service/power.h>

typedef enum {
    SceneEventPowerUsbConnectionEvent = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
    FuriPubSubSubscription* power_subscription;
} SettingsScenePowerUnplugUsb;

static void system_settings_scene_power_usb_event_callback(const void* message, void* context) {
    furi_check(message);
    furi_check(context);

    PowerEvent* event = (PowerEvent*)message;
    SystemSettings* instance = context;

    if(event->type == PowerEventUsbConnectionStateUpdate) {
        system_settings_send_custom_event(instance, SceneEventPowerUsbConnectionEvent);
    }
}

static void system_settings_scene_power_unplug_usb_on_enter(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;

    SettingsScenePowerUnplugUsb* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerUnplugUsb);

    scene->power_subscription = furi_pubsub_subscribe(
        power_get_pubsub(instance->power),
        system_settings_scene_power_usb_event_callback,
        instance);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("warning_front_8x8.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("error_back_11x11.bin"),
    };

    with_gui(instance->gui, {
        nav_bar_push_location(instance->back_nav_bar, "SHUT DOWN");
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Unplug USB cable\nto continue");
        }
    });
}

static void system_settings_scene_power_unplug_usb_on_exit(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SettingsScenePowerUnplugUsb* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerUnplugUsb);

    furi_pubsub_unsubscribe(power_get_pubsub(instance->power), scene->power_subscription);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->back_nav_bar);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool
    system_settings_scene_power_unplug_usb_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventPowerUsbConnectionEvent) {
            if(!power_is_usb_connected(instance->power)) {
                scene_manager_next_scene(instance->scene_manager, SceneIdPowerShutDownConfirm);
                consumed = true;
            }
        }
    }

    return consumed;
}

const Scene system_settings_scene_power_unplug_usb = {
    .enter_callback = system_settings_scene_power_unplug_usb_on_enter,
    .exit_callback = system_settings_scene_power_unplug_usb_on_exit,
    .event_callback = system_settings_scene_power_unplug_usb_on_event,
    .data_size = sizeof(SettingsScenePowerUnplugUsb),
};
