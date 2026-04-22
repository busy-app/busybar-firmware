#include "../clock_i.h"

#include <gui/modules/var_item_list.h>

typedef enum {
    ClockSceneSetupEventChange = ClockEventSceneEventsStart,
} ClockSceneSetupEvent;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;

    FuriMutex* settings_mutex;
} ClockSceneSetup;

static void clock_scene_setup_show_date_callback(VarItem* item, void* context) {
    Clock* instance = context;
    ClockSceneSetup* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxSetup);

    furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
    instance->settings.show_date = var_item_get_value(item);
    furi_mutex_release(scene->settings_mutex);

    clock_internal_fire_event(instance, ClockSceneSetupEventChange);
}

static void clock_scene_setup_show_seconds_callback(VarItem* item, void* context) {
    Clock* instance = context;
    ClockSceneSetup* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxSetup);

    furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
    instance->settings.show_seconds = var_item_get_value(item);
    furi_mutex_release(scene->settings_mutex);

    clock_internal_fire_event(instance, ClockSceneSetupEventChange);
}

static void clock_scene_setup_blink_colons_callback(VarItem* item, void* context) {
    Clock* instance = context;
    ClockSceneSetup* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxSetup);

    furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
    instance->settings.blink_colons = var_item_get_value(item);
    furi_mutex_release(scene->settings_mutex);

    clock_internal_fire_event(instance, ClockSceneSetupEventChange);
}

static void clock_scene_setup_on_enter(void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneSetup* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxSetup);

    scene->settings_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_list = var_item_list_alloc(instance->front_scene_window);

        VarItem* front_show_date_item = var_item_list_add_switch(
            scene->front_list, "Show date", clock_scene_setup_show_date_callback, instance);
        var_item_set_value(front_show_date_item, instance->settings.show_date);

        VarItem* front_show_seconds_item = var_item_list_add_switch(
            scene->front_list, "Show\nseconds", clock_scene_setup_show_seconds_callback, instance);
        var_item_set_value(front_show_seconds_item, instance->settings.show_seconds);

        VarItem* front_blink_colons_item = var_item_list_add_switch(
            scene->front_list, "Colon blink", clock_scene_setup_blink_colons_callback, instance);
        var_item_set_value(front_blink_colons_item, instance->settings.blink_colons);

        /* back layout setup */
        scene->back_list = var_item_list_alloc(instance->back_scene_window);

        VarItem* back_show_date_item =
            var_item_list_add_switch(scene->back_list, "Show date", NULL, NULL);
        var_item_set_value(back_show_date_item, instance->settings.show_date);

        VarItem* back_show_seconds_item =
            var_item_list_add_switch(scene->back_list, "Show seconds", NULL, NULL);
        var_item_set_value(back_show_seconds_item, instance->settings.show_seconds);

        VarItem* back_blink_colons_item =
            var_item_list_add_switch(scene->back_list, "Colon blink", NULL, NULL);
        var_item_set_value(back_blink_colons_item, instance->settings.blink_colons);
    });
}

static void clock_scene_setup_on_exit(void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneSetup* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxSetup);

    furi_mutex_free(scene->settings_mutex);

    with_gui(instance->gui, {
        var_item_list_free(scene->back_list);
        var_item_list_free(scene->front_list);
    });
}

static bool clock_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    Clock* instance = context;
    ClockSceneSetup* scene =
        scene_manager_get_scene_data(instance->scene_manager, ClockSceneIdxSetup);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ClockSceneSetupEventChange:
            furi_mutex_acquire(scene->settings_mutex, FuriWaitForever);
            ClockSettings settings = instance->settings;
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

const Scene clock_internal_scene_setup = {
    .enter_callback = clock_scene_setup_on_enter,
    .exit_callback = clock_scene_setup_on_exit,
    .event_callback = clock_scene_setup_on_event,
    .data_size = sizeof(ClockSceneSetup),
};
