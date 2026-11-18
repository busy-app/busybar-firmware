#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <storage/storage.h>
#include <gui/modules/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} JsAppLauncherSceneStatus;

typedef struct {
    const char* front_icon;
    const char* back_icon;
} JsAppLauncherSceneStatusTypeConfig;

static const JsAppLauncherSceneStatusTypeConfig js_app_launcher_scene_status_type_configs[] = {
    [JsAppLauncherShowStatusTypeGeneralError] =
        {
            .front_icon = SHARED_IMG_PATH("error_front_8x8.image"),
            .back_icon = SHARED_IMG_PATH("error_back_11x11.image"),
        },
    [JsAppLauncherShowStatusTypeGeneralInfo] =
        {
            .front_icon = SHARED_IMG_PATH("info_front_8x8.image"),
            .back_icon = SHARED_IMG_PATH("info_back_11x11.image"),
        },
};

static_assert(
    COUNT_OF(js_app_launcher_scene_status_type_configs) == JsAppLauncherShowStatusTypesCount);

static void js_app_launcher_scene_status_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneStatus* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdStatus);

    const JsAppLauncherShowStatusDesc* desc = js_app_launcher_get_show_status_desc(instance);
    const JsAppLauncherSceneStatusTypeConfig* config =
        &js_app_launcher_scene_status_type_configs[desc->type];

    with_gui(instance->gui, {
        widget_set_visible(nav_bar_get_base(instance->nav_bar), false);

        data->front_status = status_view_alloc(instance->front_window);
        status_view_set_primary_text(data->front_status, desc->primary.front);
        status_view_set_icon(data->front_status, config->front_icon, false);

        data->back_status = status_view_alloc(instance->back_window);
        status_view_set_primary_text(data->back_status, desc->primary.back);
        status_view_set_auxiliary_text(data->back_status, desc->auxiliary.back);
        status_view_set_icon(data->back_status, config->back_icon, false);
    });
}

static void js_app_launcher_scene_status_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneStatus* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdStatus);

    with_gui(instance->gui, {
        status_view_free(data->front_status);
        status_view_free(data->back_status);

        widget_set_visible(nav_bar_get_base(instance->nav_bar), true);
    });
}

static bool js_app_launcher_scene_status_on_event(const SceneManagerEvent* event, void* context) {
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

const Scene js_app_launcher_scene_status = {
    .data_size = sizeof(JsAppLauncherSceneStatus),
    .enter_callback = js_app_launcher_scene_status_on_enter,
    .exit_callback = js_app_launcher_scene_status_on_exit,
    .event_callback = js_app_launcher_scene_status_on_event,
};
