#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/label.h>
#include <gui/modules/image.h>

#define SCENE_DISPLAY_TIMEOUT_MS (3000)

typedef enum {
    SceneEventBleConnectedTimeoutExpiredEvent = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    FlexLayout* front_flex;
    Image* front_mark;
    Label* front_label;

    FlexLayout* back_flex;
    Image* back_mark;
    Label* back_label;

    FuriTimer* timer;
} BleSettingsConnectedSceneData;

static void scene_timer_callback(void* context) {
    BleSettings* instance = context;
    ble_settings_send_custom_event(instance, SceneEventBleConnectedTimeoutExpiredEvent);
}

static void scene_connected_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsConnectedSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnected);

    with_gui(instance->gui, {
        const char* label_text = "Connected";

        //Front screen
        data->front_flex = flex_layout_alloc(instance->front_scene_window, FlexLayoutTypeRow);
        flex_layout_set_spacing(data->front_flex, 2);
        widget_set_height_content(flex_layout_get_base(data->front_flex));
        widget_set_align(flex_layout_get_base(data->front_flex), AlignLeftMid);

        data->front_mark = image_alloc(flex_layout_get_base(data->front_flex));
        image_set_source(data->front_mark, SETTINGS_IMG_PATH("checkmark_front_8x6.bin"));
        widget_set_height_content(image_get_base(data->front_mark));

        data->front_label = label_alloc(flex_layout_get_base(data->front_flex));
        label_set_text(data->front_label, label_text);
        widget_set_height_content(label_get_base(data->front_label));

        //Back screen
        data->back_flex = flex_layout_alloc(instance->back_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_align(
            data->back_flex, FlexLayoutAlignCenter, FlexLayoutAlignCenter, FlexLayoutAlignCenter);

        data->back_mark = image_alloc(flex_layout_get_base(data->back_flex));
        image_set_source(data->back_mark, SETTINGS_IMG_PATH("checkmark_back_12x10.bin"));
        widget_set_width_content(image_get_base(data->back_mark));

        data->back_label = label_alloc(flex_layout_get_base(data->back_flex));
        label_set_text(data->back_label, label_text);
        widget_set_width_content(label_get_base(data->back_label));
        widget_set_margin(label_get_base(data->back_label), 0, 0, 8, 0);
    });

    data->timer = furi_timer_alloc(scene_timer_callback, FuriTimerTypeOnce, context);
    furi_timer_start(data->timer, furi_ms_to_ticks(SCENE_DISPLAY_TIMEOUT_MS));
}

static void scene_connected_on_exit(void* context) {
    furi_assert(context);

    BleSettings* instance = context;

    BleSettingsConnectedSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnected);

    with_gui(instance->gui, {
        image_free(data->front_mark);
        label_free(data->front_label);
        flex_layout_free(data->front_flex);
    });
    furi_timer_free(data->timer);
}

static bool scene_connected_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BleSettings* instance = context;
    BleSettingsConnectedSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnected);

    bool go_back = false;
    if(event->type == SceneManagerEventTypeCustom) {
        go_back = (event->event == SceneEventBleConnectedTimeoutExpiredEvent);
    } else if(event->type == SceneManagerEventTypeBack) {
        furi_timer_stop(data->timer);
        go_back = true;
    }

    bool consumed = false;
    if(go_back) {
        consumed =
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene ble_scene_connected = {
    .enter_callback = scene_connected_on_enter,
    .exit_callback = scene_connected_on_exit,
    .event_callback = scene_connected_on_event,
    .data_size = sizeof(BleSettingsConnectedSceneData),
};
