#include "../apps_menu_i.h"
#include "../storage_macros.h"

#include <desktop/desktop.h>

#include <gui/modules/image.h>
#include <gui/modules/label.h>

#include <gui/modules/menu.h>

typedef enum {
    SceneCustomEventMenuItemClicked = AppsMenuCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef enum {
    AppsSceneMainMenuIndexClock,

    AppsSceneMainMenuIndexesCount,
} AppsSceneMainMenuIndex;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    _Atomic AppsSceneMainMenuIndex menu_idx;
} AppsMenuSceneMain;

static const char* apps_menu_scene_app_names[AppsSceneMainMenuIndexesCount] = {
    [AppsSceneMainMenuIndexClock] = "clock",
};

static void apps_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(context);

    AppsMenu* instance = context;
    AppsMenuSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->menu_idx = index;
    uint32_t event = SceneCustomEventMenuItemClicked;
    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

static void apps_menu_scene_main_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        // front:
        data->front_menu = menu_alloc(instance->front_scene_window);
        menu_add_item(
            data->front_menu,
            "Clock",
            "",
            APPS_MENU_IMG_PATH("clock_front_8x8.bin"),
            AppsSceneMainMenuIndexClock,
            apps_scene_setup_menu_callback,
            instance);
        menu_set_selected_item_index(data->front_menu, data->menu_idx);

        // back:
        data->back_menu = menu_alloc(instance->back_scene_window);
        menu_add_item(
            data->back_menu,
            "CLOCK",
            "",
            APPS_MENU_IMG_PATH("clock_back_12x12.bin"),
            AppsSceneMainMenuIndexClock,
            NULL,
            instance);

        menu_set_selected_item_index(data->back_menu, data->menu_idx);
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
    });
}

static void apps_menu_scene_main_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool apps_menu_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            furi_check(data->menu_idx < AppsSceneMainMenuIndexesCount);

            Desktop* desktop = furi_record_open(RECORD_DESKTOP);
            desktop_replace_current_app(desktop, apps_menu_scene_app_names[data->menu_idx], NULL);
            furi_record_close(RECORD_DESKTOP);
        }
    }

    return false;
}

const Scene apps_menu_scene_main = {
    .enter_callback = apps_menu_scene_main_on_enter,
    .exit_callback = apps_menu_scene_main_on_exit,
    .event_callback = apps_menu_scene_main_on_event,
    .data_size = sizeof(AppsMenuSceneMain),
};
