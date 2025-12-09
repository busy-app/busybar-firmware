#include "../busy_i.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_image.h>

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    size_t menu_idx;
} BusySceneSetup;

typedef enum {
    BusySceneSetupMenuIndexTimer,
    BusySceneSetupMenuIndexTheme,
    BusySceneSetupMenuIndexMax,
} BusySceneSetupMenuIndex;

static void busy_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(index < BusySceneSetupMenuIndexMax);
    furi_assert(context);

    BusyApp* instance = context;
    busy_send_custom_event(instance, index);
}

static void busy_scene_setup_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetup);

    BusyTimerConfig timer_config;
    busy_timer_get_config(instance->busy_timer, &timer_config);

    const char* mode_name = busy_timer_get_mode_names()[timer_config.mode];

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_window);

        menu_add_item(
            data->front_menu,
            "Timer",
            mode_name,
            BUSY_IMG_PATH("timer_8x8.bin"),
            BusySceneSetupMenuIndexTimer,
            busy_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Theme",
            "",
            BUSY_IMG_PATH("theme_8x8.bin"),
            BusySceneSetupMenuIndexTheme,
            busy_scene_setup_menu_callback,
            instance);

        menu_set_selected_item_index(data->front_menu, data->menu_idx);

        data->back_menu = menu_alloc(instance->back_window);
        menu_add_item(
            data->back_menu, "TIMER", mode_name, BUSY_IMG_PATH("timer_12x12.bin"), 0, NULL, NULL);
        menu_add_item(
            data->back_menu, "THEME", "", BUSY_IMG_PATH("theme_12x12.bin"), 0, NULL, NULL);

        menu_set_selected_item_index(data->back_menu, data->menu_idx);
    });
}

static void busy_scene_setup_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetup);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool busy_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetup);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        data->menu_idx = event->event;

        if(event->event == BusySceneSetupMenuIndexTimer) {
            busy_push_location(instance, "TIMER");
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdSetupTimer);

        } else if(event->event == BusySceneSetupMenuIndexTheme) {
            busy_push_location(instance, "THEME");
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdSetupTheme);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        data->menu_idx = 0;

        busy_pop_location(instance);
    }

    return consumed;
}

const Scene busy_scene_setup = {
    .enter_callback = busy_scene_setup_on_enter,
    .exit_callback = busy_scene_setup_on_exit,
    .event_callback = busy_scene_setup_on_event,
    .data_size = sizeof(BusySceneSetup),
};
