#include "../wifi_settings.h"
#include "../widgets/wifi_state_view.h"
#include <settings_helpers/gui_params.h>

#include <wifi/wifi.h>

typedef enum {
    SceneEventOpenMenu = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    WifiStateView* back_view;
    WifiStateView* front_view;
} SceneWifiState;

static bool wifi_scene_state_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    WifiSettings* instance = context;

    bool consumed = false;
    SceneEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneEventOpenMenu;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        wifi_settings_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void wifi_scene_state_update_info(WifiSettings* instance, SceneWifiState* data) {
    WifiModelState wifi_state = wifi_model_get_state(instance->model);
    bool is_connected = (wifi_state == WifiModelStateConnected);

    FuriString* ssid_str = furi_string_alloc();
    wifi_model_get_ssid(instance->model, ssid_str);

    with_gui(instance->gui, {
        wifi_state_view_set_state(data->back_view, is_connected, furi_string_get_cstr(ssid_str));
        wifi_state_view_set_state(data->front_view, is_connected, furi_string_get_cstr(ssid_str));
    });
    furi_string_free(ssid_str);
}

static void wifi_scene_state_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiState* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdState);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, wifi_scene_state_input_callback, instance);

        data->back_view = wifi_state_view_back_alloc(instance->back_scene_window);
        data->front_view = wifi_state_view_front_alloc(instance->front_scene_window);
    });
    wifi_scene_state_update_info(instance, data);
}

static void wifi_scene_state_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    SceneWifiState* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdState);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, wifi_scene_state_input_callback);

        wifi_state_view_free(data->back_view);
        wifi_state_view_free(data->front_view);
    });
}

static bool wifi_scene_state_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiState* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdState);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppEventWifiStateChange:
            WifiModelState wifi_state = wifi_model_get_state(instance->model);
            if(wifi_state == WifiModelStateNotConfigured) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdNotConnected);
            } else {
                wifi_scene_state_update_info(instance, data);
            }
            consumed = true;
            break;
        case SceneEventOpenMenu:
            scene_manager_next_scene(instance->scene_manager, SceneIdMenu);
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

const Scene wifi_scene_state = {
    .enter_callback = wifi_scene_state_on_enter,
    .exit_callback = wifi_scene_state_on_exit,
    .event_callback = wifi_scene_state_on_event,
    .data_size = sizeof(SceneWifiState),
};
