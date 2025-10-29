#include "../debug_app_list.h"

#include <applications.h>
#include <desktop/desktop.h>
#include <gui/modules/submenu.h>
#include <settings_helpers/gui_params.h>

typedef enum {
    SceneEventMenuItemClicked = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    Submenu* submenus[GuiDisplayIdMax];
    Desktop* desktop;

    _Atomic size_t menu_idx;
} SettingsSceneDebugApps;

static void scene_main_submenu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    DebugAppList* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    scene->menu_idx = index;
    debug_app_list_send_custom_event(app, SceneEventMenuItemClicked);
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);
    DebugAppList* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);
        FuriString* app_name = furi_string_alloc();

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* window = (display == GuiDisplayIdFront) ? app->front_scene_window :
                                                              app->back_scene_window;
            scene->submenus[display] = submenu_alloc(window);

            for(uint32_t i = 0; i < FLIPPER_DEBUG_APPS_COUNT; i++) {
                const FlipperInternalApplication* debug_app = &FLIPPER_DEBUG_APPS[i];
                SubmenuItemCallback callback = scene_main_submenu_item_callback;
                furi_string_set_str(app_name, debug_app->name);

                if(display == GuiDisplayIdFront) {
                    callback = NULL;
                } else if(display == GuiDisplayIdBack) {
                    furi_string_to_upper_in_place(app_name);
                } else {
                    furi_crash();
                }

                submenu_add_item(
                    scene->submenus[display], furi_string_get_cstr(app_name), i, callback, app);
            }
        }

        furi_string_free(app_name);
    });

    scene->desktop = furi_record_open(RECORD_DESKTOP);
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);
    DebugAppList* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    furi_record_close(RECORD_DESKTOP);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            submenu_free(scene->submenus[display]);
        }
    });
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    DebugAppList* app = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventMenuItemClicked) {
            SettingsSceneDebugApps* scene =
                scene_manager_get_current_scene_data(app->scene_manager);
            const FlipperInternalApplication* app = &FLIPPER_DEBUG_APPS[scene->menu_idx];

            // TODO: make the launched app use our navbar
            // TODO: return to the Settings app chain in the same state
            desktop_replace_current_app(scene->desktop, app->name, NULL);

            consumed = true;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(app->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene debug_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(SettingsSceneDebugApps),
};
