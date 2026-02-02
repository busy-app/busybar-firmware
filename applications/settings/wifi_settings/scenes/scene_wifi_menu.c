#include "../wifi_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

#include <wifi/wifi.h>

typedef enum {
    SceneEventMenuInfo = AppEventSceneEventsStart,
    SceneEventMenuForget,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} SceneWifiMenu;

void wifi_scene_menu_item_callback(uint32_t index, void* context) {
    wifi_settings_send_custom_event(context, index);
}

static void wifi_scene_menu_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiMenu* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMenu);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "View IP address",
            SceneEventMenuInfo,
            wifi_scene_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Forget network",
            SceneEventMenuForget,
            wifi_scene_menu_item_callback,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "View IP address", SceneEventMenuInfo, NULL, instance);
        submenu_add_item(data->back_menu, "Forget network", SceneEventMenuForget, NULL, instance);
    });
}

static void wifi_scene_menu_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiMenu* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMenu);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool wifi_scene_menu_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppEventWifiStateChange:
            WifiModelState wifi_state = wifi_model_get_state(instance->model);
            if(wifi_state == WifiModelStateNotConfigured) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdNotConnected);
            }
            break;
        case SceneEventMenuInfo:
            scene_manager_next_scene(instance->scene_manager, SceneIdInfo);
            break;
        case SceneEventMenuForget:
            scene_manager_next_scene(instance->scene_manager, SceneIdForget);
            break;
        default:
            break;
        }
    }

    return consumed;
}

const Scene wifi_scene_menu = {
    .enter_callback = wifi_scene_menu_on_enter,
    .exit_callback = wifi_scene_menu_on_exit,
    .event_callback = wifi_scene_menu_on_event,
    .data_size = sizeof(SceneWifiMenu),
};
