#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <storage/storage.h>
#include <gui/modules/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} JsAppLauncherSceneError;

static void js_app_launcher_scene_error_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneError* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdError);

    const JsAppLauncherErrorDesc* desc = js_app_launcher_get_error_desc(instance);

    with_gui(instance->gui, {
        widget_set_visible(nav_bar_get_base(instance->nav_bar), false);

        data->front_status = status_view_alloc(instance->front_window);
        status_view_set_primary_text(data->front_status, desc->primary.front);
        status_view_set_icon(data->front_status, SHARED_IMG_PATH("error_front_8x8.image"), false);

        data->back_status = status_view_alloc(instance->back_window);
        status_view_set_primary_text(data->back_status, desc->primary.back);
        status_view_set_auxiliary_text(data->back_status, desc->auxiliary.back);
        status_view_set_icon(data->back_status, SHARED_IMG_PATH("error_back_11x11.image"), false);
    });
}

static void js_app_launcher_scene_error_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneError* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdError);

    with_gui(instance->gui, {
        status_view_free(data->front_status);
        status_view_free(data->back_status);

        widget_set_visible(nav_bar_get_base(instance->nav_bar), true);
    });
}

static bool js_app_launcher_scene_error_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;
    JsAppLauncher* instance = context;

    if(event->type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, JsAppLauncherSceneIdStart);
    }

    return consumed;
}

const Scene js_app_launcher_scene_error = {
    .data_size = sizeof(JsAppLauncherSceneError),
    .enter_callback = js_app_launcher_scene_error_on_enter,
    .exit_callback = js_app_launcher_scene_error_on_exit,
    .event_callback = js_app_launcher_scene_error_on_event,
};
