#include "../firmware_i.h"

#include <gui/modules/status_view.h>

typedef enum {
    FirmwareSettingsLowBatterySceneEventChargeAmountUpdate = FirmwareSettingsEventSceneEventsStart,
    FirmwareSettingsLowBatterySceneEventUsbConnectionStateUpdate,
} FirmwareSettingsLowBatterySceneEvent;

typedef struct {
    StatusView* front_status;
    StatusView* back_status;

    FuriPubSubSubscription* power_event_subscription;
} FirmwareSettingsLowBatteryScene;

typedef enum {
    FirmwareSettingsLowBatteryScenePresetIdxUsbConnected,
    FirmwareSettingsLowBatteryScenePresetIdxUsbDisconnected,

    FirmwareSettingsLowBatteryScenePresetIdxsCount
} FirmwareSettingsLowBatteryScenePresetIdx;

typedef struct {
    const char* front_image_path;
    const char* front_text;

    const char* back_primary_text;
    const char* back_auxiliary_text;
} FirmwareSettingsLowBatteryScenePreset;

static const FirmwareSettingsLowBatteryScenePreset firmware_settings_low_battery_scene_presets[];

static inline FirmwareSettingsLowBatteryScene*
    firmware_settings_low_battery_scene_get(FirmwareSettings* instance) {
    return scene_manager_get_scene_data(
        instance->scene_manager, FirmwareSettingsSceneIdxLowBattery);
}

static void
    firmware_settings_low_battery_scene_power_event_callback(const void* message, void* context) {
    const PowerEvent* event = message;
    FirmwareSettings* instance = context;

    switch(event->type) {
    case PowerEventChargeAmountUpdate:
        firmware_settings_internal_fire_event(
            instance, FirmwareSettingsLowBatterySceneEventChargeAmountUpdate);
        break;

    case PowerEventUsbConnectionStateUpdate:
        firmware_settings_internal_fire_event(
            instance, FirmwareSettingsLowBatterySceneEventUsbConnectionStateUpdate);
        break;

    default:
        break;
    }
}

static void firmware_settings_low_battery_scene_on_usb_connection_state_update(
    FirmwareSettings* instance) {
    FirmwareSettingsLowBatteryScene* scene = firmware_settings_low_battery_scene_get(instance);

    const FirmwareSettingsLowBatteryScenePreset* scene_preset =
        &firmware_settings_low_battery_scene_presets
            [power_is_usb_connected(instance->power) ?
                 FirmwareSettingsLowBatteryScenePresetIdxUsbConnected :
                 FirmwareSettingsLowBatteryScenePresetIdxUsbDisconnected];

    with_gui(instance->gui, {
        status_view_set_icon(scene->front_status, scene_preset->front_image_path, false);
        status_view_set_primary_text(scene->front_status, scene_preset->front_text);

        status_view_set_primary_text(scene->back_status, scene_preset->back_primary_text);
        status_view_set_auxiliary_text(scene->back_status, scene_preset->back_auxiliary_text);
    });
}

static void firmware_settings_low_battery_scene_on_enter(void* context) {
    FirmwareSettings* instance = context;
    FirmwareSettingsLowBatteryScene* scene = firmware_settings_low_battery_scene_get(instance);

    scene->power_event_subscription = furi_pubsub_subscribe(
        power_get_pubsub(instance->power),
        firmware_settings_low_battery_scene_power_event_callback,
        instance);

    const FirmwareSettingsLowBatteryScenePreset* scene_preset =
        &firmware_settings_low_battery_scene_presets
            [power_is_usb_connected(instance->power) ?
                 FirmwareSettingsLowBatteryScenePresetIdxUsbConnected :
                 FirmwareSettingsLowBatteryScenePresetIdxUsbDisconnected];

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

static void firmware_settings_low_battery_scene_on_exit(void* context) {
    FirmwareSettings* instance = context;
    FirmwareSettingsLowBatteryScene* scene = firmware_settings_low_battery_scene_get(instance);

    furi_pubsub_unsubscribe(power_get_pubsub(instance->power), scene->power_event_subscription);

    with_gui(instance->gui, {
        status_view_free(scene->back_status);
        status_view_free(scene->front_status);
    });
}

static bool
    firmware_settings_low_battery_scene_on_event(const SceneManagerEvent* event, void* context) {
    FirmwareSettings* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case FirmwareSettingsLowBatterySceneEventChargeAmountUpdate:
            if(updater_get_allowance_status(instance->updater) != UpdaterStatusBatteryLow) {
                scene_manager_previous_scene(instance->scene_manager);
            }
            return true;

        case FirmwareSettingsLowBatterySceneEventUsbConnectionStateUpdate:
            firmware_settings_low_battery_scene_on_usb_connection_state_update(instance);
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene firmware_settings_internal_scene_low_battery = {
    .enter_callback = firmware_settings_low_battery_scene_on_enter,
    .exit_callback = firmware_settings_low_battery_scene_on_exit,
    .event_callback = firmware_settings_low_battery_scene_on_event,
    .data_size = sizeof(FirmwareSettingsLowBatteryScene),
};

static const FirmwareSettingsLowBatteryScenePreset firmware_settings_low_battery_scene_presets[] = {
    [FirmwareSettingsLowBatteryScenePresetIdxUsbConnected] =
        {
            /* front layout */
            .front_image_path = THIS_IMG_PATH("charging_battery_front_8x8.image"),
            .front_text = "Update will start\nat 40% charge",

            /* back layout */
            .back_primary_text = "Battery charging...",
            .back_auxiliary_text = "Update will start at 40%",
        },

    [FirmwareSettingsLowBatteryScenePresetIdxUsbDisconnected] =
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
    COUNT_OF(firmware_settings_low_battery_scene_presets) ==
    FirmwareSettingsLowBatteryScenePresetIdxsCount);
