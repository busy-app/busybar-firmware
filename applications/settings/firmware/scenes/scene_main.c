#include "../firmware_i.h"

#include <gui/modules/submenu.h>

typedef enum {
    ThisSceneEventCheckForUpdate = ThisEventSceneEventsStart,
    ThisSceneEventSettings,
} ThisSceneEvent;

typedef struct {
    Submenu* front_submenu;
    Submenu* back_submenu;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxMain);
}

static void this_submenu_item_callback(uint32_t index, void* context) {
    settings_firmware_app_fire_event(context, index);
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_submenu = submenu_alloc(instance->front_scene_window);

        submenu_add_item(
            scene->front_submenu,
            "Check for update",
            ThisSceneEventCheckForUpdate,
            this_submenu_item_callback,
            instance);

        submenu_add_item(
            scene->front_submenu,
            "Settings",
            ThisSceneEventSettings,
            this_submenu_item_callback,
            instance);

        /* back layout setup */
        scene->back_submenu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(scene->back_submenu, "Check for update", 0, NULL, NULL);
        submenu_add_item(scene->back_submenu, "Settings", 0, NULL, NULL);
    });
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    with_gui(instance->gui, {
        submenu_free(scene->back_submenu);
        submenu_free(scene->front_submenu);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisSceneEventCheckForUpdate:
            scene_manager_next_scene(instance->scene_manager, ThisSceneIdxCheck);
            return true;

        case ThisSceneEventSettings:
            scene_manager_next_scene(instance->scene_manager, ThisSceneIdxSettings);

            with_gui(instance->gui, {
                nav_bar_push_location(instance->back_nav_bar, "SETTINGS");
            });
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene settings_firmware_app_scene_main = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
