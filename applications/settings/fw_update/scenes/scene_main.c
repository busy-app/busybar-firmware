#include "../fw_update.h"

#include <gui/modules/submenu.h>

#include <settings_helpers/gui_params.h>

typedef enum {
    SceneEventBackPressed = AppEventSceneEventsStart,

    SceneEventCheckForUpdate,
    SceneEventSettings
} SceneEvent;

typedef struct {
    Submenu* front_submenu;
    Submenu* back_submenu;
} FwUpdateScene;

void scene_submenu_item_callback(uint32_t index, void* context) {
    fw_update_send_custom_event(context, index);
}

static void scene_on_enter(void* context) {
    furi_assert(context);

    FwUpdate* instance = context;
    FwUpdateScene* scene = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        /* front submenu setup */
        scene->front_submenu = submenu_alloc(instance->front_scene_window);

        submenu_add_item(
            scene->front_submenu,
            "Check for update",
            SceneEventCheckForUpdate,
            scene_submenu_item_callback,
            instance);

        submenu_add_item(
            scene->front_submenu,
            "Settings",
            SceneEventSettings,
            scene_submenu_item_callback,
            instance);

        /* back submenu setup */
        scene->back_submenu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(scene->back_submenu, "Check for update", 0, NULL, NULL);
        submenu_add_item(scene->back_submenu, "Settings", 0, NULL, NULL);
    });
}

static void scene_on_exit(void* context) {
    furi_assert(context);

    FwUpdate* instance = context;
    FwUpdateScene* scene = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        submenu_free(scene->front_submenu);
        submenu_free(scene->back_submenu);
    });
}

static bool scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FwUpdate* instance = context;

    bool is_consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventBackPressed:
            scene_manager_handle_back_event(instance->scene_manager);
            is_consumed = true;
            break;

        case SceneEventCheckForUpdate:
            break;

        case SceneEventSettings:
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return is_consumed;
}

const Scene fw_update_scene_main = {
    .enter_callback = scene_on_enter,
    .exit_callback = scene_on_exit,
    .event_callback = scene_on_event,
    .data_size = sizeof(FwUpdateScene),
};
