
#include "../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

#include <power/power_service/power.h>

typedef enum {
    SceneEventShutDown = AppEventSceneEventsStart,
    SceneEventRestart,
    SceneEventInfo,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
    uint32_t menu_index;
} SettingsSceneSystem;

static void scene_power_menu_menu_item_callback(uint32_t index, void* context) {
    system_settings_send_custom_event(context, index);
}

static void scene_power_menu_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerMenu);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "Shut down",
            SceneEventShutDown,
            scene_power_menu_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Restart device",
            SceneEventRestart,
            scene_power_menu_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Info",
            SceneEventInfo,
            scene_power_menu_menu_item_callback,
            instance);
        submenu_set_selected_item_index(data->front_menu, data->menu_index);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "Shut down", SceneEventShutDown, NULL, instance);
        submenu_add_item(data->back_menu, "Restart device", SceneEventRestart, NULL, instance);
        submenu_add_item(data->back_menu, "Info", SceneEventInfo, NULL, instance);
        submenu_set_selected_item_index(data->back_menu, data->menu_index);
    });
}

static void scene_power_menu_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerMenu);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool scene_power_menu_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerMenu);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventShutDown) {
            bool is_usb_connected = power_is_usb_connected(instance->power);
            system_settings_push_location(instance, "SHUT DOWN");
            if(is_usb_connected) {
                scene_manager_next_scene(instance->scene_manager, SceneIdPowerUnplugUsb);
            } else {
                scene_manager_next_scene(instance->scene_manager, SceneIdPowerShutDownConfirm);
            }
            consumed = true;
        } else if(event->event == SceneEventRestart) {
            system_settings_push_location(instance, "RESTART");
            scene_manager_next_scene(instance->scene_manager, SceneIdPowerRestartConfirm);
            consumed = true;
        } else if(event->event == SceneEventInfo) {
            system_settings_push_location(instance, "INFO");
            scene_manager_next_scene(instance->scene_manager, SceneIdPowerInfo);
            consumed = true;
        }

        data->menu_index = event->event;
    } else if(event->type == SceneManagerEventTypeBack) {
        system_settings_pop_location(instance);
        data->menu_index = SceneEventShutDown;
        scene_manager_previous_scene(instance->scene_manager);
        consumed = true;
    }

    return consumed;
}

const Scene system_settings_scene_power_menu = {
    .enter_callback = scene_power_menu_on_enter,
    .exit_callback = scene_power_menu_on_exit,
    .event_callback = scene_power_menu_on_event,
    .data_size = sizeof(SettingsSceneSystem),
};
