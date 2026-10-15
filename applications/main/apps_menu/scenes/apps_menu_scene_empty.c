#include "../apps_menu_i.h"
#include "apps_menu_scenes.h"

#include <storage/storage.h>

#include <gui/modules/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} AppsMenuSceneEmpty;

static void apps_menu_scene_empty_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneEmpty* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdEmpty);

    with_gui(instance->gui, {
        data->front_status = status_view_alloc(instance->front_scene_window);
        data->back_status = status_view_alloc(instance->back_scene_window);

        status_view_set_primary_text(data->front_status, "No apps yet, go to\nhttp://10.0.4.20");
        status_view_set_icon(data->front_status, SHARED_IMG_PATH("info_front_8x8.image"), false);

        status_view_set_primary_text(data->back_status, "No apps yet. Add via http://10.0.4.20");
        status_view_set_icon(data->back_status, SHARED_IMG_PATH("error_back_11x11.image"), false);
    });
}

static void apps_menu_scene_empty_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneEmpty* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdEmpty);

    with_gui(instance->gui, {
        status_view_free(data->front_status);
        status_view_free(data->back_status);
    });
}

static bool apps_menu_scene_empty_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    AppsMenu* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, AppsMenuSceneIdStart);
    }

    return consumed;
}

const Scene apps_menu_scene_empty = {
    .enter_callback = apps_menu_scene_empty_on_enter,
    .exit_callback = apps_menu_scene_empty_on_exit,
    .event_callback = apps_menu_scene_empty_on_event,
    .data_size = sizeof(AppsMenuSceneEmpty),
};
