#include "../wifi_settings_i.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} SceneDisconnecting;

static void wifi_scene_disconnecting_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneDisconnecting* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdDisconnecting);

    with_gui(instance->gui, {
        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(data->front_status, SHARED_ANIM_PATH("spinner_front_8x8.anim"), true);
        status_view_set_primary_text(data->front_status, "Disconnecting...");

        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(data->back_status, SHARED_ANIM_PATH("spinner_back_16x16.anim"), true);
        status_view_set_primary_text(data->back_status, "Disconnecting...");
    });

    wifi_model_forget(instance->model);
    desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
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
    UNUSED(event);
    UNUSED(context);

    return true;
}

const Scene wifi_scene_disconnecting = {
    .enter_callback = wifi_scene_disconnecting_on_enter,
    .exit_callback = wifi_scene_disconnecting_on_exit,
    .event_callback = wifi_scene_disconnecting_on_event,
    .data_size = sizeof(SceneDisconnecting),
};
