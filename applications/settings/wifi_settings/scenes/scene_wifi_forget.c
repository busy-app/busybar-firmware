#include "../wifi_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/dialog.h>

#include <wifi/wifi.h>

typedef enum {
    SceneEventConfirm = AppEventSceneEventsStart,
    SceneEventCancel,
} SceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} SceneWifiForget;

static void wifi_scene_forget_callback(uint8_t result, void* context) {
    WifiSettings* instance = context;
    if(result == 0) {
        wifi_settings_send_custom_event(instance, SceneEventConfirm);
    } else {
        wifi_settings_send_custom_event(instance, SceneEventCancel);
    }
}

static void wifi_scene_forget_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiForget* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdForget);

    with_gui(instance->gui, {
        data->front_dialog = dialog_alloc(instance->front_scene_window);
        data->back_dialog = dialog_alloc(instance->back_scene_window);

        dialog_set_text(data->front_dialog, "Forget network?");
        dialog_set_text(data->back_dialog, "Forget network?");

        Color color_forget = COLOR_MAKE_RGB(0xED, 0x00, 0x18);
        Color color_cancel = COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF);
        dialog_set_option_colors(data->front_dialog, color_forget, color_cancel);

        dialog_set_options(data->front_dialog, "Forget", "Cancel");
        dialog_set_options(data->back_dialog, "Forget", "Cancel");

        dialog_set_calback(data->front_dialog, wifi_scene_forget_callback, instance);
    });
}

static void wifi_scene_forget_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiForget* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdForget);

    with_gui(instance->gui, {
        dialog_free(data->front_dialog);
        dialog_free(data->back_dialog);
    });
}

static bool wifi_scene_forget_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventConfirm:
            wifi_model_forget(instance->model);
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
            consumed = true;
            break;
        case SceneEventCancel:
            scene_manager_previous_scene(instance->scene_manager);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

const Scene wifi_scene_forget = {
    .enter_callback = wifi_scene_forget_on_enter,
    .exit_callback = wifi_scene_forget_on_exit,
    .event_callback = wifi_scene_forget_on_event,
    .data_size = sizeof(SceneWifiForget),
};
