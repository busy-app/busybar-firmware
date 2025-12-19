#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>
#include <gui/modules/label.h>

#include "../widgets/split_select_widget.h"

typedef enum {
    SceneEventRemovePairingConfirm = AppEventSceneEventsStart,
    SceneEventRemovePairingCancel
} BleSettingsForgetDeviceConfirmSceneEvent;

typedef struct {
    SplitWidget* split;
} BleSettingsForgetConfirmSceneData;

static void scene_main_menu_item_callback(uint32_t index, void* context) {
    ble_settings_send_custom_event(context, index);
}

static void scene_forget_device_confirm_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsForgetConfirmSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdForgetDeviceConfirm);

    with_gui(instance->gui, {
        data->split = split_widget_alloc(instance->front_scene_window, "Forget device?");

        split_widget_add_button(
            data->split,
            "Forget",
            SceneEventRemovePairingConfirm,
            scene_main_menu_item_callback,
            instance);

        split_widget_add_button(
            data->split,
            "Cancel",
            SceneEventRemovePairingCancel,
            scene_main_menu_item_callback,
            instance);
    });
}

static void scene_forget_device_confirm_on_exit(void* context) {
    BleSettings* instance = context;
    BleSettingsForgetConfirmSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdForgetDeviceConfirm);
    with_gui(instance->gui, { split_widget_free(data->split); });
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
