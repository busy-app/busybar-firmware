#include "../account_settings.h"
#include <settings_helpers/gui_params.h>
#include <settings_helpers/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} SceneConnecting;

static void account_scene_connecting_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneConnecting* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnecting);

    with_gui(instance->gui, {
        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(data->front_status, SETTINGS_IMG_PATH("spinner_front_7x7.bin"));
        status_view_set_header(data->front_status, "Connecting...");

        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(data->back_status, SETTINGS_IMG_PATH("spinner_back_16x16.bin"));
        status_view_set_header(data->back_status, "Connecting...");
    });
}

static void account_scene_connecting_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    SceneConnecting* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdConnecting);

    with_gui(instance->gui, {
        status_view_free(data->back_status);
        status_view_free(data->front_status);
    });
}

static bool account_scene_connecting_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppEventAccountStateChange:
            AccountModelState state = account_model_get_state(instance->model);
            if(state == AccountModelStateConnectedNotLinked) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdNotLinked);
            } else if(state == AccountModelStateConnectedLinked) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdLinked);
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

const Scene account_scene_connecting = {
    .enter_callback = account_scene_connecting_on_enter,
    .exit_callback = account_scene_connecting_on_exit,
    .event_callback = account_scene_connecting_on_event,
    .data_size = sizeof(SceneConnecting),
};
