#include "../system_settings.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/status_view.h>

typedef enum {
    SystemSettingsLowBatterySceneEventChargeAmountUpdate = AppEventSceneEventsStart,
    SystemSettingsLowBatterySceneEventUsbConnectionStateUpdate,
} SystemSettingsLowBatterySceneEvent;

typedef struct {
    StatusView* front_status;
    StatusView* back_status;

    FuriPubSubSubscription* power_event_subscription;
} SystemSettingsLowBatteryScene;

typedef enum {
    SystemSettingsLowBatteryScenePresetIdxUsbConnected,
    SystemSettingsLowBatteryScenePresetIdxUsbDisconnected,

    SystemSettingsLowBatteryScenePresetIdxsCount
} SystemSettingsLowBatteryScenePresetIdx;

typedef struct {
    const char* front_image_path;
    const char* front_text;

    const char* back_primary_text;
    const char* back_auxiliary_text;
} SystemSettingsLowBatteryScenePreset;

static const SystemSettingsLowBatteryScenePreset system_settings_low_battery_scene_presets[];

static void
    system_settings_low_battery_scene_power_event_callback(const void* message, void* context) {
    const PowerEvent* event = message;
    SystemSettings* instance = context;

    switch(event->type) {
    case PowerEventChargeAmountUpdate:
        system_settings_send_custom_event(
            instance, SystemSettingsLowBatterySceneEventChargeAmountUpdate);
        break;

    case PowerEventUsbConnectionStateUpdate:
        system_settings_send_custom_event(
            instance, SystemSettingsLowBatterySceneEventUsbConnectionStateUpdate);
        break;

    default:
        break;
    }
}

static void
    system_settings_low_battery_scene_on_usb_connection_state_update(SystemSettings* instance) {
    SystemSettingsLowBatteryScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLowBattery);

    const SystemSettingsLowBatteryScenePreset* scene_preset =
        &system_settings_low_battery_scene_presets
            [power_is_usb_connected(instance->power) ?
                 SystemSettingsLowBatteryScenePresetIdxUsbConnected :
                 SystemSettingsLowBatteryScenePresetIdxUsbDisconnected];

    with_gui(instance->gui, {
        status_view_set_icon(scene->front_status, scene_preset->front_image_path, false);
        status_view_set_primary_text(scene->front_status, scene_preset->front_text);

        status_view_set_primary_text(scene->back_status, scene_preset->back_primary_text);
        status_view_set_auxiliary_text(scene->back_status, scene_preset->back_auxiliary_text);
    });
}

static void system_settings_low_battery_scene_on_enter(void* context) {
    SystemSettings* instance = context;
    SystemSettingsLowBatteryScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLowBattery);

    scene->power_event_subscription = furi_pubsub_subscribe(
        power_get_pubsub(instance->power),
        system_settings_low_battery_scene_power_event_callback,
        instance);

    const SystemSettingsLowBatteryScenePreset* scene_preset =
        &system_settings_low_battery_scene_presets
            [power_is_usb_connected(instance->power) ?
                 SystemSettingsLowBatteryScenePresetIdxUsbConnected :
                 SystemSettingsLowBatteryScenePresetIdxUsbDisconnected];

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(scene->front_status, scene_preset->front_image_path, false);
        status_view_set_primary_text(scene->front_status, scene_preset->front_text);

        /* back layout setup */
        scene->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(scene->back_status, SHARED_IMG_PATH("error_back_11x11.image"), false);
        status_view_set_primary_text(scene->back_status, scene_preset->back_primary_text);
        status_view_set_auxiliary_text(scene->back_status, scene_preset->back_auxiliary_text);
    });
}

static void system_settings_low_battery_scene_on_exit(void* context) {
    SystemSettings* instance = context;
    SystemSettingsLowBatteryScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLowBattery);

    furi_pubsub_unsubscribe(power_get_pubsub(instance->power), scene->power_event_subscription);

    with_gui(instance->gui, {
        status_view_free(scene->back_status);
        status_view_free(scene->front_status);
    });
}

static bool
    system_settings_low_battery_scene_on_event(const SceneManagerEvent* event, void* context) {
    SystemSettings* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SystemSettingsLowBatterySceneEventChargeAmountUpdate:
            if(updater_get_allowance_status(instance->updater) != UpdaterStatusBatteryLow) {
                scene_manager_previous_scene(instance->scene_manager);
            }
            return true;

        case SystemSettingsLowBatterySceneEventUsbConnectionStateUpdate:
            system_settings_low_battery_scene_on_usb_connection_state_update(instance);
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene system_settings_internal_scene_low_battery = {
    .enter_callback = system_settings_low_battery_scene_on_enter,
    .exit_callback = system_settings_low_battery_scene_on_exit,
    .event_callback = system_settings_low_battery_scene_on_event,
    .data_size = sizeof(SystemSettingsLowBatteryScene),
};

static const SystemSettingsLowBatteryScenePreset system_settings_low_battery_scene_presets[] = {
    [SystemSettingsLowBatteryScenePresetIdxUsbConnected] =
        {
            /* front layout */
            .front_image_path = SHARED_IMG_PATH("charging_battery_front_8x8.image"),
            .front_text = "Charging to 40%\nto continue",

            /* back layout */
            .back_primary_text = "Battery charging...",
            .back_auxiliary_text = "You can continue at 40%",
        },

    [SystemSettingsLowBatteryScenePresetIdxUsbDisconnected] =
        {
            /* front layout */
            .front_image_path = SHARED_IMG_PATH("low_battery_front_8x8.image"),
            .front_text = "Charge device up\nto 40% to update",

            /* back layout */
            .back_primary_text = "Charge your BUSY Bar",
            .back_auxiliary_text = "40% needed to start update",
        },
};

static_assert(
    COUNT_OF(system_settings_low_battery_scene_presets) ==
    SystemSettingsLowBatteryScenePresetIdxsCount);
