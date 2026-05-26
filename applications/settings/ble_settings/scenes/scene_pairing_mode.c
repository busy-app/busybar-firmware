#include "../ble_settings.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/status_view.h>

#include <ble/ble.h>

#define TAG "BlePairScene"

#define NAME_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_HEX(0x888888))
#define STATUS_LIGHTS_COLOR   ((Color)COLOR_MAKE_RGB(0, 0, 0xFF))

typedef enum {
    SceneEventBlePairingEvent = AppEventSceneEventsStart,
    SceneEventDeviceNameChangedEvent,
    SceneEventBlePairingTimeout,
} SceneEvent;

typedef struct {
    StatusView* front_status;
    StatusView* back_status;

    FuriString* ble_name;
    FuriString* auxiliary_text_builder;
} BleSettingsPairingSceneData;

static void scene_pairing_model_changed_callback(BleModelStateEvent event, void* context) {
    BleSettings* instance = context;

    furi_check(event < BleModelStateEventCount);
    const SceneEvent model_to_scene_events[BleModelStateEventCount] = {
        [BleModelStateEventBleChanged] = SceneEventBlePairingEvent,
        [BleModelStateEventNameChanged] = SceneEventDeviceNameChangedEvent,
        [BleModelStateEventPairingTimeout] = SceneEventBlePairingTimeout,
    };

    SceneEvent scene_event = model_to_scene_events[event];
    ble_settings_send_custom_event(instance, scene_event);
}

static void scene_pairing_mode_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsPairingSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

    ble_model_set_state_callback(instance->model, scene_pairing_model_changed_callback, instance);

    data->ble_name = furi_string_alloc();
    data->auxiliary_text_builder = furi_string_alloc();

    ble_model_get_name(instance->model, data->ble_name);
    furi_string_printf(
        data->auxiliary_text_builder, "Device name: %s", furi_string_get_cstr(data->ble_name));

    ble_model_start(instance->model);

    with_gui(instance->gui, {
        /* front layout setup */
        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(data->front_status, ANIM_PATH("ble_pairing_8x8.anim"), true);
        status_view_set_primary_text(data->front_status, "Pairing mode...");

        /* back layout setup */
        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(data->back_status, IMG_PATH("ble_back_white_11x11.image"), false);
        status_view_set_primary_text(data->back_status, "Pairing mode...");
        status_view_set_auxiliary_text(
            data->back_status, furi_string_get_cstr(data->auxiliary_text_builder));
    });

    brightness_control_set_brightness_override(
        instance->brightness_control, BrightnessControlModuleStatusLights, BRIGHTNESS_MAX);
    status_lights_run_preset(
        instance->status_lights, StatusLightsPresetBlink, STATUS_LIGHTS_COLOR);
}

static void scene_pairing_mode_on_exit(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsPairingSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

    if(!ble_model_is_device_paired(instance->model)) {
        ble_model_stop(instance->model);
    }

    ble_model_set_state_callback(instance->model, NULL, NULL);

    with_gui(instance->gui, {
        status_view_free(data->back_status);
        status_view_free(data->front_status);
    });

    furi_string_free(data->auxiliary_text_builder);
    furi_string_free(data->ble_name);

    status_lights_run_preset(instance->status_lights, StatusLightsPresetOff, (Color){});
    brightness_control_reset_brightness_override(
        instance->brightness_control, BrightnessControlModuleStatusLights);
}

static bool scene_pairing_mode_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BleSettings* instance = context;

    bool consumed = false;
    bool exit = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventBlePairingEvent) {
            if(ble_model_is_device_paired(instance->model)) {
                scene_manager_next_scene(instance->scene_manager, SceneIdConnected);
                consumed = true;
            }
        } else if(event->event == SceneEventDeviceNameChangedEvent) {
            BleSettingsPairingSceneData* data =
                scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

            ble_model_get_name(instance->model, data->ble_name);
            furi_string_printf(
                data->auxiliary_text_builder,
                "Device name: %s",
                furi_string_get_cstr(data->ble_name));

            with_gui(instance->gui, {
                status_view_set_auxiliary_text(
                    data->back_status, furi_string_get_cstr(data->auxiliary_text_builder));
            });
            consumed = true;
        } else if(event->event == SceneEventBlePairingTimeout) {
            exit = true;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        exit = true;
    }

    if(exit) {
        consumed =
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene ble_scene_pairing_mode = {
    .enter_callback = scene_pairing_mode_on_enter,
    .exit_callback = scene_pairing_mode_on_exit,
    .event_callback = scene_pairing_mode_on_event,
    .data_size = sizeof(BleSettingsPairingSceneData),
};
