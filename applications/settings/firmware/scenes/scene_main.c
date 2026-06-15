#include "../firmware_i.h"

#include <gui/modules/submenu.h>

typedef enum {
    FirmwareSettingsMainSceneEventCheckForUpdate = FirmwareSettingsEventSceneEventsStart,
    FirmwareSettingsMainSceneEventSettings,
} FirmwareSettingsMainSceneEvent;

typedef struct {
    Submenu* front_submenu;
    Submenu* back_submenu;
} FirmwareSettingsMainScene;

static inline FirmwareSettingsMainScene*
    firmware_settings_main_scene_get(FirmwareSettings* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxMain);
}

static void firmware_settings_main_scene_submenu_callback(uint32_t index, void* context) {
    firmware_settings_internal_fire_event(context, index);
}

static void firmware_settings_main_scene_on_enter(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsMainScene* scene = firmware_settings_main_scene_get(instance);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_submenu = submenu_alloc(instance->front_scene_window);

        submenu_add_item(
            scene->front_submenu,
            "Check for update",
            NULL,
            FirmwareSettingsMainSceneEventCheckForUpdate,
            firmware_settings_main_scene_submenu_callback,
            instance);

        submenu_add_item(
            scene->front_submenu,
            "Settings",
            NULL,
            FirmwareSettingsMainSceneEventSettings,
            firmware_settings_main_scene_submenu_callback,
            instance);

        /* back layout setup */
        scene->back_submenu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(scene->back_submenu, "Check for update", NULL, 0, NULL, NULL);
        submenu_add_item(scene->back_submenu, "Settings", NULL, 0, NULL, NULL);

        widget_set_scrollbar_enabled(submenu_get_base(scene->front_submenu), true);
        widget_set_scrollbar_enabled(submenu_get_base(scene->back_submenu), true);
    });
}

static void firmware_settings_main_scene_on_exit(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsMainScene* scene = firmware_settings_main_scene_get(instance);

    with_gui(instance->gui, {
        submenu_free(scene->back_submenu);
        submenu_free(scene->front_submenu);
    });
}

static bool firmware_settings_main_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case FirmwareSettingsMainSceneEventCheckForUpdate:
            scene_manager_next_scene(instance->scene_manager, FirmwareSettingsSceneIdxCheck);
            return true;

        case FirmwareSettingsMainSceneEventSettings:
            scene_manager_next_scene(instance->scene_manager, FirmwareSettingsSceneIdxSettings);

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

const Scene firmware_settings_internal_scene_main = {
    .enter_callback = firmware_settings_main_scene_on_enter,
    .exit_callback = firmware_settings_main_scene_on_exit,
    .event_callback = firmware_settings_main_scene_on_event,
    .data_size = sizeof(FirmwareSettingsMainScene),
};
