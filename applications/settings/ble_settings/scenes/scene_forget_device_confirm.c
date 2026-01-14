#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/dialog.h>

typedef enum {
    SceneEventRemovePairingConfirm = AppEventSceneEventsStart,
    SceneEventRemovePairingCancel
} BleSettingsForgetDeviceConfirmSceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} BleSettingsForgetConfirmSceneData;

static void scene_forget_device_dialog_callback(uint8_t result, void* context) {
    BleSettings* instance = context;
    ble_settings_send_custom_event(
        instance, (result == 0) ? SceneEventRemovePairingConfirm : SceneEventRemovePairingCancel);
}

static void scene_forget_device_confirm_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsForgetConfirmSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdForgetDeviceConfirm);

    with_gui(instance->gui, {
        data->front_dialog = dialog_alloc(instance->front_scene_window);
        data->back_dialog = dialog_alloc(instance->back_scene_window);

        dialog_set_text(data->front_dialog, "Forget device?");
        dialog_set_text(data->back_dialog, "Forget device?");

        Color color_forget = COLOR_MAKE_RGB(0xED, 0x00, 0x18);
        Color color_cancel = COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF);
        dialog_set_option_colors(data->front_dialog, color_forget, color_cancel);

        dialog_set_options(data->front_dialog, "Forget", "Cancel");
        dialog_set_options(data->back_dialog, "Forget", "Cancel");

        dialog_set_calback(data->front_dialog, scene_forget_device_dialog_callback, instance);
    });
}

static void scene_forget_device_confirm_on_exit(void* context) {
    BleSettings* instance = context;
    BleSettingsForgetConfirmSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdForgetDeviceConfirm);
    with_gui(instance->gui, {
        dialog_free(data->front_dialog);
        dialog_free(data->back_dialog);
    });
}

static bool scene_forget_device_confirm_on_event(const SceneManagerEvent* event, void* context) {
    BleSettings* instance = context;
    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventRemovePairingConfirm) {
            if(ble_forget(instance->ble))
                consumed = desktop_replace_current_app(
                    instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        } else if(event->event == SceneEventRemovePairingCancel) {
            consumed = scene_manager_previous_scene(instance->scene_manager);
        }
    }

    return consumed;
}

const Scene ble_scene_forget_device_confirm = {
    .enter_callback = scene_forget_device_confirm_on_enter,
    .exit_callback = scene_forget_device_confirm_on_exit,
    .event_callback = scene_forget_device_confirm_on_event,
    .data_size = sizeof(BleSettingsForgetConfirmSceneData),
};
