#include "../settings.h"

#include <applications.h>
#include <desktop/desktop.h>
#include <gui/modules/submenu.h>

typedef struct {
    Submenu* submenus[GuiDisplayIdMax];
    Desktop* desktop;
} SettingsSceneDebugApps;

static void settings_scene_debug_apps_submenu_item_callback(uint32_t index, void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    settings_send_custom_event(app, index);
}

static void settings_scene_debug_apps_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
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
                SubmenuItemCallback callback = settings_scene_debug_apps_submenu_item_callback;
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

static void settings_scene_debug_apps_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    furi_record_close(RECORD_DESKTOP);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            submenu_free(scene->submenus[display]);
        }
    });
}

static bool settings_scene_debug_apps_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        uint32_t app_index = event->event;
        furi_check(app_index < FLIPPER_DEBUG_APPS_COUNT);
        const FlipperInternalApplication* app = &FLIPPER_DEBUG_APPS[app_index];

        // TODO: make the launched app use our navbar
        // TODO: return to the Settings app in the same state
        desktop_replace_current_app(scene->desktop, app->name, NULL);

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(app);
    }

    return consumed;
}

const Scene settings_scene_debug_apps = {
    .enter_callback = settings_scene_debug_apps_on_enter,
    .exit_callback = settings_scene_debug_apps_on_exit,
    .event_callback = settings_scene_debug_apps_on_event,
    .data_size = sizeof(SettingsSceneDebugApps),
};
