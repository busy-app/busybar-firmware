#include "../firmware_i.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>

#define BACK_DETAIL_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88))

typedef enum {
    FirmwareSettingsLowBatterySceneEventChargeAmountUpdate = FirmwareSettingsEventSceneEventsStart,
    FirmwareSettingsLowBatterySceneEventUsbConnectionStateUpdate,
} FirmwareSettingsLowBatterySceneEvent;

typedef struct {
    FlexBox* front_box;
    Image* front_image;
    Label* front_label;

    FlexBox* back_box;
    Label* back_primary_label;

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
        image_set_source(scene->front_image, scene_preset->front_image_path);
        label_set_text(scene->front_label, scene_preset->front_text);

        label_set_text(scene->back_primary_label, scene_preset->back_primary_text);
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
        scene->front_box = flex_box_alloc(instance->front_scene_window);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        scene->front_image = image_alloc(flex_box_get_base(scene->front_box));
        image_set_source(scene->front_image, scene_preset->front_image_path);

        scene->front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_line_spacing(scene->front_label, 0);
        label_set_text(scene->front_label, scene_preset->front_text);

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_scene_window);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->back_box, 3);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        Image* back_image = image_alloc(flex_box_get_base(scene->back_box));
        image_set_source(back_image, SHARED_IMG_PATH("error_back_11x11.bin"));
        widget_set_padding(image_get_base(back_image), 0, 0, 2, 7);

        scene->back_primary_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(scene->back_primary_label, scene_preset->back_primary_text);
        label_set_text_align(scene->back_primary_label, TextAlignCenter);

        Label* back_detail_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text_color(back_detail_label, BACK_DETAIL_LABEL_TEXT_COLOR);
        label_set_text(back_detail_label, "40% needed to start update");
    });
}

static void firmware_settings_low_battery_scene_on_exit(void* context) {
    FirmwareSettings* instance = context;
    FirmwareSettingsLowBatteryScene* scene = firmware_settings_low_battery_scene_get(instance);

    furi_pubsub_unsubscribe(power_get_pubsub(instance->power), scene->power_event_subscription);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
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
            .front_image_path = THIS_IMG_PATH("charging_battery_front_8x8.bin"),
            .front_text = "Charging to 40%\nto start update...",

            /* back layout */
            .back_primary_text = "Battery is charging...",
        },

    [FirmwareSettingsLowBatteryScenePresetIdxUsbDisconnected] =
        {
            /* front layout */
            .front_image_path = THIS_IMG_PATH("low_battery_front_8x8.bin"),
            .front_text = "Charge device up\nto 40% to update",

            /* back layout */
            .back_primary_text = "Charge your BUSY Bar",
        },
};

static_assert(
    COUNT_OF(firmware_settings_low_battery_scene_presets) ==
    FirmwareSettingsLowBatteryScenePresetIdxsCount);
