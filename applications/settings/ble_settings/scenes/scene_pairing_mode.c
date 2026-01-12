#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/anim_image.h>
#include <gui/modules/label.h>

#include <ble/ble.h>

typedef enum {
    SceneEventBlePairingEvent = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    FlexLayout* flex;
    AnimImage* front_anim;
    Label* front_label;
    FuriPubSubSubscription* pubsub_subscription;
} SettingsSceneBle;

static void scene_pairing_ble_pairing_done_callback(const void* message, void* context) {
    UNUSED(message);
    BleSettings* instance = context;
    ble_settings_send_custom_event(instance, SceneEventBlePairingEvent);
}

static bool ble_settings_is_device_paired(Ble* ble) {
    BleStatus status = {0};
    ble_get_status(ble, &status);
    FURI_LOG_I("BleSettings", "Pair status: %d", status.pairing);
    return status.pairing == BlePairingStatePaired;
}

static void scene_pairing_mode_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    SettingsSceneBle* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

    data->pubsub_subscription = furi_pubsub_subscribe(
        ble_get_pubsub(instance->ble), scene_pairing_ble_pairing_done_callback, context);

    ///TODO: Rework this to enable BLE on start according to previous state, and move it out of here
    ble_start(instance->ble);

    with_gui(instance->gui, {
        data->flex = flex_layout_alloc(instance->front_scene_window, FlexLayoutTypeRow);
        flex_layout_set_spacing(data->flex, 2);
        widget_set_height_content(flex_layout_get_base(data->flex));
        widget_set_align(flex_layout_get_base(data->flex), AlignLeftMid);

        data->front_anim = anim_image_alloc(flex_layout_get_base(data->flex));
        anim_image_set_source(data->front_anim, ANIM_PATH("ble_pairing_8x8.anim"));
        anim_image_set_range(data->front_anim, 0, 3, true, false);
        widget_set_height_content(anim_image_get_base(data->front_anim));

        data->front_label = label_alloc(flex_layout_get_base(data->flex));
        label_set_text(data->front_label, "Pairing mode...");
        widget_set_height_content(label_get_base(data->front_label));
    });
}

static void scene_pairing_mode_on_exit(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    SettingsSceneBle* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

    furi_pubsub_unsubscribe(ble_get_pubsub(instance->ble), data->pubsub_subscription);

    with_gui(instance->gui, {
        anim_image_free(data->front_anim);
        label_free(data->front_label);
        flex_layout_free(data->flex);
    });

    status_lights_run_preset(instance->status_lights, StatusLightsPresetOff, (Color){});
}

static bool scene_pairing_mode_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BleSettings* instance = context;

    bool go_back = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventBlePairingEvent) {
            go_back = ble_settings_is_device_paired(instance->ble);
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        go_back = true;
    }

    bool consumed = false;
    if(go_back) {
        consumed =
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene ble_scene_pairing_mode = {
    .enter_callback = scene_pairing_mode_on_enter,
    .exit_callback = scene_pairing_mode_on_exit,
    .event_callback = scene_pairing_mode_on_event,
    .data_size = sizeof(SettingsSceneBle),
};
