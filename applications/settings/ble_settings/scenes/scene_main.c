#include "../ble_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

#include <ble/ble.h>

typedef enum {
    SceneEventBleEnable = AppEventSceneEventsStart,
    SceneEventBleDisable,
    SceneEventRemovePairing,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} SettingsSceneBle;

void scene_main_menu_item_callback(uint32_t index, void* context) {
    ble_settings_send_custom_event(context, index);
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    SettingsSceneBle* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "Enable",
            SceneEventBleEnable,
            scene_main_menu_item_callback,
            instance);

        submenu_add_item(
            data->front_menu,
            "Disable",
            SceneEventBleDisable,
            scene_main_menu_item_callback,
            instance);

        submenu_add_item(
            data->front_menu,
            "Forget pairing",
            SceneEventRemovePairing,
            scene_main_menu_item_callback,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "Enable", SceneEventBleEnable, NULL, instance);

        submenu_add_item(data->back_menu, "Disable", SceneEventBleDisable, NULL, instance);

        submenu_add_item(
            data->back_menu, "Forget pairing", SceneEventRemovePairing, NULL, instance);
    });
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);

    BleSettings* instance = context;
    SettingsSceneBle* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });

    status_lights_run_preset(instance->status_lights, StatusLightsPresetOff, (Color){});
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BleSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        Ble* ble = furi_record_open(RECORD_BLE);
        switch(event->event) {
        case SceneEventBleEnable:
            consumed = ble_start(ble);
            break;
        case SceneEventBleDisable:
            consumed = ble_stop(ble);
            break;
        case SceneEventRemovePairing:
            consumed = ble_forget(ble);
            break;
        default:
            break;
        }
        furi_record_close(RECORD_BLE);

    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene ble_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(SettingsSceneBle),
};
