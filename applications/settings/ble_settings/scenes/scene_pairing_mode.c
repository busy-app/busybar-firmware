#include "../ble_settings.h"
#include "../widgets/named_label_view.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/anim_image.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>

#include <ble/ble.h>

#define TAG "BlePairScene"

#define NAME_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_HEX(0x444444))
#define STATUS_LIGHTS_COLOR   ((Color)COLOR_MAKE_RGB(0, 0, 0xFF))

typedef enum {
    SceneEventBlePairingEvent = AppEventSceneEventsStart,
    SceneEventDeviceNameChangedEvent,
} SceneEvent;

typedef struct {
    FlexLayout* front_flex;
    AnimImage* front_anim;
    Label* front_label;

    FlexLayout* back_flex;
    Image* back_image;
    Label* back_label;
    NamedLabelView* name_view;

    FuriString* name_label_text;
} BleSettingsPairingSceneData;

static void scene_pairing_model_changed_callback(BleModelStateEvent event, void* context) {
    BleSettings* instance = context;

    furi_check(event < BleModelStateEventCount);
    const SceneEvent model_to_scene_events[BleModelStateEventCount] = {
        [BleModelStateEventBleChanged] = SceneEventBlePairingEvent,
        [BleModelStateEventNameChanged] = SceneEventDeviceNameChangedEvent,
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

    data->name_label_text = furi_string_alloc();
    ble_model_get_name(instance->model, data->name_label_text);

    ble_model_start(instance->model);

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
        widget_set_size_content(image_get_base(data->back_image));
        widget_set_margin(image_get_base(data->back_image), 0, 0, 0, 6);

        data->back_label = label_alloc(flex_layout_get_base(data->back_flex));
        widget_set_width_content(label_get_base(data->back_label));
        label_set_text(data->back_label, pairing_text);
        widget_set_margin(label_get_base(data->back_label), 0, 0, 0, 2);

        data->name_view = named_label_view_back_alloc(flex_layout_get_base(data->back_flex));
        named_label_set_title(data->name_view, "Device name: ");
        named_label_set_text(data->name_view, furi_string_get_cstr(data->name_label_text));
        named_label_set_text_color(data->name_view, NAME_LABEL_TEXT_COLOR);
    });

    status_lights_set_brightness(instance->status_lights, STATUS_LIGHTS_BRIGHTNESS_MAX);
    status_lights_run_preset(instance->status_lights, StatusLightsPresetFade, STATUS_LIGHTS_COLOR);
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
        anim_image_free(data->front_anim);
        label_free(data->front_label);
        flex_layout_free(data->front_flex);

        image_free(data->back_image);
        label_free(data->back_label);
        named_label_view_back_free(data->name_view);
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
            if(ble_model_is_device_paired(instance->model)) {
                scene_manager_next_scene(instance->scene_manager, SceneIdConnected);
            }
        } else if(event->event == SceneEventDeviceNameChangedEvent) {
            BleSettingsPairingSceneData* data =
                scene_manager_get_scene_data(instance->scene_manager, SceneIdPairingMode);
            ble_model_get_name(instance->model, data->name_label_text);
            with_gui(instance->gui, {
                named_label_set_text(data->name_view, furi_string_get_cstr(data->name_label_text));
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
