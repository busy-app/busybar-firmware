#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/anim_image.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>

#include <ble/ble.h>

#define NAME_LABEL_ANIMATION_DURATION_MS (3000)
#define NAME_LABEL_TEXT_COLOR            ((Color)COLOR_MAKE_HEX(0x444444))
#define STATUS_LIGHTS_COLOR              ((Color)COLOR_MAKE_RGB(0, 0, 0xFF))

#define DEVICE_NAME_UPDATE_LAYOUT_DELAY_MS (50)

typedef enum {
    SceneEventBlePairingEvent = AppEventSceneEventsStart,
    SceneEventDeviceNameChangedEvent,
    SceneEventUpdateLayoutEvent,

} SceneEvent;

typedef struct {
    FlexLayout* front_flex;
    AnimImage* front_anim;
    Label* front_label;

    FlexLayout* back_flex;
    Image* back_image;
    Label* back_label;

    FlexLayout* name_flex;
    Label* title_label;
    Label* name_label;

    FuriPubSubSubscription* ble_pubsub;
    FuriPubSubSubscription* device_name_pubsub;
    FuriString* name_label_text;
} BleSettingsPairingSceneData;

static void scene_pairing_ble_pairing_done_callback(const void* message, void* context) {
    UNUSED(message);
    BleSettings* instance = context;
    ble_settings_send_custom_event(instance, SceneEventBlePairingEvent);
}

static void scene_pairing_device_name_change_callback(const void* message, void* context) {
    UNUSED(message);
    BleSettings* instance = context;
    ble_settings_send_custom_event(instance, SceneEventDeviceNameChangedEvent);
}

static void scene_pairing_mode_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsPairingSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

    data->ble_pubsub = furi_pubsub_subscribe(
        ble_get_pubsub(instance->ble), scene_pairing_ble_pairing_done_callback, context);

    data->device_name_pubsub = furi_pubsub_subscribe(
        device_name_get_pubsub(instance->device_name),
        scene_pairing_device_name_change_callback,
        context);

    data->name_label_text = furi_string_alloc();
    device_name_get(instance->device_name, data->name_label_text);

    ///TODO: Rework this to enable BLE on start according to previous state, and move it out of here
    ble_start(instance->ble);

    with_gui(instance->gui, {
        const char* pairing_text = "Pairing mode...";
        //Front screen
        data->front_flex = flex_layout_alloc(instance->front_scene_window, FlexLayoutTypeRow);
        flex_layout_set_spacing(data->front_flex, 2);
        widget_set_height_content(flex_layout_get_base(data->front_flex));
        widget_set_align(flex_layout_get_base(data->front_flex), AlignLeftMid);

        data->front_anim = anim_image_alloc(flex_layout_get_base(data->front_flex));
        anim_image_set_source(data->front_anim, ANIM_PATH("ble_pairing_8x8.anim"));
        anim_image_set_range(data->front_anim, 0, 3, true, false);
        widget_set_height_content(anim_image_get_base(data->front_anim));

        data->front_label = label_alloc(flex_layout_get_base(data->front_flex));
        label_set_text(data->front_label, pairing_text);
        widget_set_height_content(label_get_base(data->front_label));

        //Back screen
        data->back_flex = flex_layout_alloc(instance->back_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_align(
            data->back_flex, FlexLayoutAlignCenter, FlexLayoutAlignCenter, FlexLayoutAlignCenter);

        data->back_image = image_alloc(flex_layout_get_base(data->back_flex));
        image_set_source(data->back_image, IMG_PATH("ble_back_white_11x11.bin"));
        widget_set_width_content(image_get_base(data->back_image));
        widget_set_margin(image_get_base(data->back_image), 0, 0, 0, 6);

        data->back_label = label_alloc(flex_layout_get_base(data->back_flex));
        widget_set_width_content(label_get_base(data->back_label));
        label_set_text(data->back_label, pairing_text);
        widget_set_margin(label_get_base(data->back_label), 0, 0, 0, 2);

        data->name_flex =
            flex_layout_alloc(flex_layout_get_base(data->back_flex), FlexLayoutTypeRow);
        widget_set_size_content(flex_layout_get_base(data->name_flex));

        data->title_label = label_alloc(flex_layout_get_base(data->name_flex));
        label_set_text(data->title_label, "Device name: ");
        widget_set_size_content(label_get_base(data->title_label));

        data->name_label = label_alloc(flex_layout_get_base(data->name_flex));
        widget_set_size_content(label_get_base(data->name_label));

        label_set_text(data->name_label, furi_string_get_cstr(data->name_label_text));
        label_set_long_content_mode(
            data->name_label, LabelLongContentModeScroll, NAME_LABEL_ANIMATION_DURATION_MS);

        label_set_text_color(data->title_label, NAME_LABEL_TEXT_COLOR);
        label_set_text_color(data->name_label, NAME_LABEL_TEXT_COLOR);
    });

    status_lights_set_brightness(instance->status_lights, STATUS_LIGHTS_BRIGHTNESS_MAX);
    status_lights_run_preset(instance->status_lights, StatusLightsPresetFade, STATUS_LIGHTS_COLOR);

    ble_settings_send_custom_event(instance, SceneEventDeviceNameChangedEvent);
}

static void scene_pairing_mode_on_exit(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsPairingSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);

    furi_pubsub_unsubscribe(ble_get_pubsub(instance->ble), data->ble_pubsub);
    furi_pubsub_unsubscribe(
        device_name_get_pubsub(instance->device_name), data->device_name_pubsub);

    with_gui(instance->gui, {
        anim_image_free(data->front_anim);
        label_free(data->front_label);
        flex_layout_free(data->front_flex);

        image_free(data->back_image);
        label_free(data->back_label);

        label_free(data->name_label);
        label_free(data->title_label);
        flex_layout_free(data->name_flex);

        flex_layout_free(data->back_flex);
    });

    furi_string_free(data->name_label_text);
    status_lights_run_preset(instance->status_lights, StatusLightsPresetOff, (Color){});
}

static bool scene_pairing_mode_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BleSettings* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventBlePairingEvent) {
            if(ble_settings_is_device_paired(instance->ble))
                scene_manager_next_scene(instance->scene_manager, SceneIdConnected);
        } else if(event->event == SceneEventDeviceNameChangedEvent) {
            BleSettingsPairingSceneData* data =
                scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);
            device_name_get(instance->device_name, data->name_label_text);
            with_gui(instance->gui, {
                label_set_text(data->name_label, furi_string_get_cstr(data->name_label_text));
            });
            furi_delay_ms(DEVICE_NAME_UPDATE_LAYOUT_DELAY_MS);
            ble_settings_send_custom_event(instance, SceneEventUpdateLayoutEvent);
        } else if(event->event == SceneEventUpdateLayoutEvent) {
            BleSettingsPairingSceneData* data =
                scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);
            with_gui(instance->gui, {
                int32_t scene_win_w = widget_get_width(instance->back_scene_window);
                int32_t name_flex_w = widget_get_width(flex_layout_get_base(data->name_flex));

                if(name_flex_w > scene_win_w) {
                    int32_t title_w = widget_get_width(label_get_base(data->title_label));
                    widget_set_width(label_get_base(data->name_label), scene_win_w - title_w);
                } else {
                    widget_set_size_content(label_get_base(data->name_label));
                }
            });
        }
    } else if(event->type == SceneManagerEventTypeBack) {
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
