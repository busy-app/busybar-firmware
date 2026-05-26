#include "../power_on_i.h"

#include <gui/modules/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} SceneStarting;

static void power_on_scene_starting_on_enter(void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneStarting* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdStarting);

    with_gui(app->gui, {
        scene->front_status = status_view_alloc(app->front_root);
        status_view_set_icon(scene->front_status, SHARED_ANIM_PATH("spinner_front_8x8.anim"));
        status_view_set_primary_text(scene->front_status, "Starting...");

        scene->back_status = status_view_alloc(app->back_root);
        status_view_set_icon(scene->back_status, SHARED_ANIM_PATH("spinner_back_16x16.anim"));
        status_view_set_primary_text(scene->back_status, "Starting...");
    });
}

static void power_on_scene_starting_on_exit(void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneStarting* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdStarting);

    with_gui(app->gui, {
        status_view_free(scene->back_status);
        status_view_free(scene->front_status);
    });
}

static bool power_on_scene_starting_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneStarting* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdStarting);
    UNUSED(scene);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case PowerOnAppEventStarted:
            if(power_on_is_done_flag_present(app)) {
                furi_event_loop_stop(app->event_loop);
            } else {
                scene_manager_replace_current_scene(app->scene_manager, SceneIdAnimation);
            }
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

const Scene power_on_scene_starting = {
    .enter_callback = power_on_scene_starting_on_enter,
    .exit_callback = power_on_scene_starting_on_exit,
    .event_callback = power_on_scene_starting_on_event,
    .data_size = sizeof(SceneStarting),
};
