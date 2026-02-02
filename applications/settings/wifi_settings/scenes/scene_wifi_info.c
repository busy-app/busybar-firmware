#include "../wifi_settings.h"
#include "../widgets/wifi_info_view.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#include <wifi/wifi.h>

typedef enum {
    SceneEventExit = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    WifiInfoView* back_view;
    WifiInfoView* front_view;
} SceneWifiInfo;

static bool wifi_scene_info_input_callback(const InputEvent* event, void* context) {
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
            custom_event = SceneEventExit;
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

static void wifi_scene_info_update_ip(WifiSettings* instance, SceneWifiInfo* data) {
    WifiModelState wifi_state = wifi_model_get_state(instance->model);
    bool is_connected = (wifi_state == WifiModelStateConnected);

    FuriString* ip_str = furi_string_alloc();
    wifi_model_get_ip(instance->model, ip_str);

    with_gui(instance->gui, {
        wifi_info_view_set_address(data->back_view, is_connected, furi_string_get_cstr(ip_str));
        wifi_info_view_set_address(data->front_view, is_connected, furi_string_get_cstr(ip_str));
    });
    furi_string_free(ip_str);
}

static void wifi_scene_info_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiInfo* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdInfo);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, wifi_scene_info_input_callback, instance);

        data->back_view = wifi_info_view_back_alloc(instance->back_scene_window);
        data->front_view = wifi_info_view_front_alloc(instance->front_scene_window);
    });
    wifi_scene_info_update_ip(instance, data);
}

static void wifi_scene_info_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    SceneWifiInfo* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdInfo);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, wifi_scene_info_input_callback);

        wifi_info_view_free(data->back_view);
        wifi_info_view_free(data->front_view);
    });
}

static bool wifi_scene_info_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiInfo* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdInfo);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppEventWifiStateChange:
            WifiModelState wifi_state = wifi_model_get_state(instance->model);
            if(wifi_state == WifiModelStateNotConfigured) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdNotConnected);
            } else {
                wifi_scene_info_update_ip(instance, data);
            }
            break;
        case SceneEventExit:
            scene_manager_previous_scene(instance->scene_manager);
            break;
        default:
            break;
        }
    }

    return consumed;
}

const Scene wifi_scene_info = {
    .enter_callback = wifi_scene_info_on_enter,
    .exit_callback = wifi_scene_info_on_exit,
    .event_callback = wifi_scene_info_on_event,
    .data_size = sizeof(SceneWifiInfo),
};
