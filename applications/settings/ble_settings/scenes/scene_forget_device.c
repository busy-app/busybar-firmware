#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventRemovePairing = AppEventSceneEventsStart,
} BleSettingsForgetDeviceSceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} BleSettingsForgetDeviceSceneData;

static void scene_main_menu_item_callback(uint32_t index, void* context) {
    ble_settings_send_custom_event(context, index);
}

static void scene_forget_device_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    BleSettingsForgetDeviceSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdForgetDevice);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);

        submenu_add_item(
            data->front_menu,
            "Forget device",
            SceneEventRemovePairing,
            scene_main_menu_item_callback,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(
            data->back_menu, "Forget device", SceneEventRemovePairing, NULL, instance);
    });
}

static void scene_forget_device_on_exit(void* context) {
    BleSettings* instance = context;
    BleSettingsForgetDeviceSceneData* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdForgetDevice);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool scene_forget_device_on_event(const SceneManagerEvent* event, void* context) {
    BleSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventRemovePairing)
            scene_manager_next_scene(instance->scene_manager, SceneIdForgetDeviceConfirm);
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene ble_scene_forget_device = {
    .enter_callback = scene_forget_device_on_enter,
    .exit_callback = scene_forget_device_on_exit,
    .event_callback = scene_forget_device_on_event,
    .data_size = sizeof(BleSettingsForgetDeviceSceneData),
};
