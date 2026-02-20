#include "../clock_i.h"
#include "../settings/settings.h"

#include <gui/modules/var_item_list.h>

typedef enum {
    ThisSceneEventChange = ThisEventSceneEventsStart,
} ThisSceneEvent;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;

    FuriMutex* settings_mutex;
    ClockSettings settings;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxSetup);
}

static void this_list_show_date_callback(VarItem* item, void* context) {
    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
    scene->settings.show_date = var_item_get_value(item);
    furi_mutex_release(scene->settings_mutex);

    clock_app_fire_event(instance, ThisSceneEventChange);
}

static void this_list_show_seconds_callback(VarItem* item, void* context) {
    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
    scene->settings.show_seconds = var_item_get_value(item);
    furi_mutex_release(scene->settings_mutex);

    clock_app_fire_event(instance, ThisSceneEventChange);
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    scene->settings_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    clock_settings_load(&scene->settings);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_list = var_item_list_alloc(instance->front_scene_window);

        VarItem* front_show_date_item = var_item_list_add_switch(
            scene->front_list, "Show date", this_list_show_date_callback, instance);
        var_item_set_value(front_show_date_item, scene->settings.show_date);

        VarItem* front_show_seconds_item = var_item_list_add_switch(
            scene->front_list, "Show seconds", this_list_show_seconds_callback, instance);
        var_item_set_value(front_show_seconds_item, scene->settings.show_seconds);

        /* back layout setup */
        scene->back_list = var_item_list_alloc(instance->back_scene_window);

        VarItem* back_show_date_item =
            var_item_list_add_switch(scene->back_list, "Show date", NULL, NULL);
        var_item_set_value(back_show_date_item, scene->settings.show_date);

        VarItem* back_show_seconds_item =
            var_item_list_add_switch(scene->back_list, "Show seconds", NULL, NULL);
        var_item_set_value(back_show_seconds_item, scene->settings.show_seconds);
    });
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    furi_mutex_free(scene->settings_mutex);

    with_gui(instance->gui, {
        var_item_list_free(scene->back_list);
        var_item_list_free(scene->front_list);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisSceneEventChange:
            furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
            ClockSettings settings = scene->settings;
            furi_mutex_release(scene->settings_mutex);

            clock_settings_save(&settings);
            return true;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        with_gui(instance->gui, { nav_bar_pop_location(instance->back_nav_bar); });
    }

    return false;
}

const Scene clock_app_scene_setup = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
