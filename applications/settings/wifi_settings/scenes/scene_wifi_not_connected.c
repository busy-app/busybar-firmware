#include "../wifi_settings.h"
#include <settings_helpers/wifi_not_connected_view.h>
#include <settings_helpers/gui_params.h>

#include <wifi/wifi.h>

typedef enum {
    SceneEventConnectDone = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    WifiNotConnectedView* front_view;
    WifiNotConnectedView* back_view;
} SceneWifiNotConnected;

static void wifi_scene_not_connected_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiNotConnected* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdNotConnected);

    with_gui(instance->gui, {
        data->front_view = wifi_not_connected_view_front_alloc(instance->front_scene_window);

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* top_back_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);
        data->back_view = wifi_not_connected_view_back_alloc(top_back_layer_root);
    });
}

static void wifi_scene_not_connected_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    SceneWifiNotConnected* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdNotConnected);

    with_gui(instance->gui, {
        wifi_not_connected_view_free(data->front_view);
        wifi_not_connected_view_free(data->back_view);
    });
}

static bool wifi_scene_not_connected_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppEventWifiStateChange:
            WifiModelState wifi_state = wifi_model_get_state(instance->model);
            if(wifi_state == WifiModelStateConnected) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdState);
            }
            consumed = true;
            break;
        default:
            break;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        consumed = true;
    }

    return consumed;
}

const Scene wifi_scene_not_connected = {
    .enter_callback = wifi_scene_not_connected_on_enter,
    .exit_callback = wifi_scene_not_connected_on_exit,
    .event_callback = wifi_scene_not_connected_on_event,
    .data_size = sizeof(SceneWifiNotConnected),
};
