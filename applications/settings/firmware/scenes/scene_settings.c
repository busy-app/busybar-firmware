#include "../firmware_i.h"

#include <gui/modules/var_item_list.h>

typedef enum {
    ThisSceneEventSettingChange = ThisEventSceneEventsStart,
} ThisSceneEvent;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;

    FuriMutex* updater_settings_mutex;
    UpdaterSettings updater_settings;
} ThisScene;

static inline ThisScene* get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxSettings);
}

static void list_autoupdate_item_callback(VarItem* item, void* context) {
    ThisInstance* instance = context;
    ThisScene* scene = get_scene(instance);

    furi_mutex_acquire(scene->updater_settings_mutex, FuriWaitForever);
    scene->updater_settings.autoupdate_enabled = var_item_get_value(item);
    furi_mutex_release(scene->updater_settings_mutex);

    settings_firmware_app_fire_event(instance, ThisSceneEventSettingChange);
}

static void scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = get_scene(instance);

    scene->updater_settings_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    updater_get_settings(instance->updater, &scene->updater_settings);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_list = var_item_list_alloc(instance->front_scene_window);

        VarItem* front_auto_update_item = var_item_list_add_switch(
            scene->front_list, "Auto-update", list_autoupdate_item_callback, instance);
        var_item_set_value(front_auto_update_item, scene->updater_settings.autoupdate_enabled);

        /* back layout setup */
        scene->back_list = var_item_list_alloc(instance->back_scene_window);

        VarItem* back_auto_update_item =
            var_item_list_add_switch(scene->back_list, "Auto-update", NULL, NULL);
        var_item_set_value(back_auto_update_item, scene->updater_settings.autoupdate_enabled);
    });
}

static void scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = get_scene(instance);

    furi_mutex_free(scene->updater_settings_mutex);

    with_gui(instance->gui, {
        var_item_list_free(scene->back_list);
        var_item_list_free(scene->front_list);
    });
}

static bool scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = get_scene(instance);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisSceneEventSettingChange:
            furi_mutex_acquire(scene->updater_settings_mutex, FuriWaitForever);
            UpdaterSettings updater_settings = scene->updater_settings;
            furi_mutex_release(scene->updater_settings_mutex);

            updater_set_settings(instance->updater, &updater_settings);
            return true;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        with_gui(instance->gui, { nav_bar_pop_location(instance->back_nav_bar); });
    }

    return false;
}

const Scene settings_firmware_internal_scene_settings = {
    .enter_callback = scene_on_enter,
    .exit_callback = scene_on_exit,
    .event_callback = scene_on_event,
    .data_size = sizeof(ThisScene),
};
