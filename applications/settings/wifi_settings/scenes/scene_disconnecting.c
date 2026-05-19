#include "../wifi_settings_i.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
    bool forget_done;
} SceneDisconnecting;

typedef enum {
    SceneEventForgetDone = AppEventSceneEventsStart,
} SceneEvent;

static void wifi_scene_disconnecting_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneDisconnecting* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdDisconnecting);
    data->forget_done = false;

    with_gui(instance->gui, {
        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(data->front_status, SHARED_ANIM_PATH("spinner_front_8x8.anim"));
        status_view_set_primary_text(data->front_status, "Disconnecting...");

        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(data->back_status, SHARED_ANIM_PATH("spinner_back_16x16.anim"));
        status_view_set_primary_text(data->back_status, "Disconnecting...");
    });

    WifiModelState wifi_state = wifi_model_get_state(instance->model);
    if(wifi_state == WifiModelStateNotConfigured || wifi_state == WifiModelStateConnected) {
        wifi_model_forget(instance->model);
        data->forget_done = true;
        wifi_settings_send_custom_event(instance, SceneEventForgetDone);
    }
}

static void wifi_scene_disconnecting_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneDisconnecting* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdDisconnecting);

    with_gui(instance->gui, {
        status_view_free(data->back_status);
        status_view_free(data->front_status);
    });
}

static bool wifi_scene_disconnecting_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneDisconnecting* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdDisconnecting);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventForgetDone:
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
            break;
        case AppEventWifiStateChange:
            if(!data->forget_done) {
                WifiModelState wifi_state = wifi_model_get_state(instance->model);
                if(wifi_state == WifiModelStateNotConfigured ||
                   wifi_state == WifiModelStateConnected) {
                    wifi_model_forget(instance->model);
                    data->forget_done = true;
                    desktop_replace_current_app(
                        instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
                }
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

const Scene wifi_scene_disconnecting = {
    .enter_callback = wifi_scene_disconnecting_on_enter,
    .exit_callback = wifi_scene_disconnecting_on_exit,
    .event_callback = wifi_scene_disconnecting_on_event,
    .data_size = sizeof(SceneDisconnecting),
};
