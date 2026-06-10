#include "../apps_menu_i.h"
#include "../storage_macros.h"
#include "apps_menu_scenes.h"
#include "../app_list.h"

#include <desktop/desktop.h>
#include <gui/modules/menu.h>

typedef enum {
    SceneCustomEventMenuItemClicked = AppsMenuCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    _Atomic AppsMenuEntryIdx menu_idx;
} AppsMenuSceneMain;

static void apps_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(context);

    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    data->menu_idx = index;
    furi_message_queue_put(
        instance->event_queue, &(uint32_t){SceneCustomEventMenuItemClicked}, FuriWaitForever);
}

static void apps_menu_scene_main_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    with_gui(instance->gui, {
        // front:
        data->front_menu = menu_alloc(instance->front_scene_window);
        menu_add_item(
            data->front_menu,
            "Clock",
            "",
            APPS_MENU_IMG_PATH("clock_front_8x8.image"),
            AppsMenuEntryIdxClock,
            apps_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Coming soon...",
            "",
            APPS_MENU_IMG_PATH("soon_front_8x8.image"),
            AppsMenuEntryIdxComingSoon,
            apps_scene_setup_menu_callback,
            instance);
        menu_set_selected_item_index(data->front_menu, data->menu_idx);

        // back:
        data->back_menu = menu_alloc(instance->back_scene_window);
        menu_add_item(
            data->back_menu,
            "Clock",
            "",
            APPS_MENU_IMG_PATH("clock_back_11x11.image"),
            0,
            NULL,
            NULL);
        menu_add_item(
            data->back_menu,
            "Coming soon...",
            "",
            APPS_MENU_IMG_PATH("soon_back_11x11.image"),
            0,
            NULL,
            NULL);
        menu_set_selected_item_index(data->back_menu, data->menu_idx);

        widget_set_scrollbar_enabled(menu_get_base(data->front_menu), true);
        widget_set_scrollbar_enabled(menu_get_base(data->back_menu), true);

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
    });
}

static void apps_menu_scene_main_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool apps_menu_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneCustomEventMenuItemClicked:
            furi_check(data->menu_idx < AppsMenuEntryIdxsCount);

            const char* target_application = apps_menu_entries[data->menu_idx];

            if(target_application) {
                strlcpy(
                    instance->settings.active_application,
                    target_application,
                    sizeof(instance->settings.active_application));
                apps_menu_settings_save(&instance->settings);

                Desktop* desktop = furi_record_open(RECORD_DESKTOP);
                desktop_replace_current_app(desktop, target_application, NULL);
                furi_record_close(RECORD_DESKTOP);
            } else {
                scene_manager_next_scene(instance->scene_manager, AppsMenuSceneIdComingSoon);
            }
            return true;

        default:
            break;
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
