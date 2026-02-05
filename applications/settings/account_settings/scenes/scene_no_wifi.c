#include "../account_settings.h"
#include <settings_helpers/gui_params.h>
#include <settings_helpers/wifi_not_connected_view.h>

typedef struct {
    WifiNotConnectedView* front_view;
    WifiNotConnectedView* back_view;
} SceneNoWifi;

static void account_scene_no_wifi_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneNoWifi* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdNoWifi);

    with_gui(instance->gui, {
        data->front_view = wifi_not_connected_view_front_alloc(instance->front_scene_window);

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* top_back_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);
        data->back_view = wifi_not_connected_view_back_alloc(top_back_layer_root);
    });
}

static void account_scene_no_wifi_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    SceneNoWifi* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdNoWifi);

    with_gui(instance->gui, {
        wifi_not_connected_view_free(data->front_view);
        wifi_not_connected_view_free(data->back_view);
    });
}

static bool account_scene_no_wifi_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        default:
            break;
        }
        // TODO: wifi connected event?
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        consumed = true;
    }

    return consumed;
}

const Scene account_scene_no_wifi = {
    .enter_callback = account_scene_no_wifi_on_enter,
    .exit_callback = account_scene_no_wifi_on_exit,
    .event_callback = account_scene_no_wifi_on_event,
    .data_size = sizeof(SceneNoWifi),
};
