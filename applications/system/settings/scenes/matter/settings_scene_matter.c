#include "../../settings.h"
#include "../settings_scenes.h"

#include <gui/modules/submenu.h>

typedef enum {
    SceneSubmenuIndexPairing,
    SceneSubmenuIndexReset,
} SceneSubmenuIndex;

typedef enum {
    SceneCustomEventMenuItemClicked = SettingsCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    Submenu* submenus[GuiDisplayIdMax];

    _Atomic size_t menu_idx;
} SettingsSceneMatter;

static void settings_scene_matter_submenu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    SettingsApp* app = context;
    SettingsSceneMatter* scene = scene_manager_get_current_scene_data(app->scene_manager);

    scene->menu_idx = index;
    settings_send_custom_event(app, SceneCustomEventMenuItemClicked);
}

static void settings_scene_matter_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneMatter* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* window = (display == GuiDisplayIdFront) ? app->front_scene_window :
                                                              app->back_scene_window;
            scene->submenus[display] = submenu_alloc(window);
            bool add_callback = display == GuiDisplayIdBack;

            submenu_add_item(
                scene->submenus[display],
                "Pair new controller",
                SceneSubmenuIndexPairing,
                add_callback ? settings_scene_matter_submenu_item_callback : NULL,
                app);
            submenu_add_item(
                scene->submenus[display],
                "Forget all pairings",
                SceneSubmenuIndexReset,
                add_callback ? settings_scene_matter_submenu_item_callback : NULL,
                app);
        }
    });
}

static void settings_scene_matter_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneMatter* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            submenu_free(scene->submenus[display]);
        }
    });
}

static bool settings_scene_matter_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* app = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            SettingsSceneMatter* scene = scene_manager_get_current_scene_data(app->scene_manager);

            if(scene->menu_idx == SceneSubmenuIndexPairing) {
                scene_manager_next_scene(app->scene_manager, SettingsAppSceneIdMatterPairing);
            } else if(scene->menu_idx == SceneSubmenuIndexReset) {
                matter_factory_reset(app->matter);
                scene_manager_next_scene(app->scene_manager, SettingsAppSceneIdReboot);
            } else {
                furi_crash();
            }

            consumed = true;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(app);
    }

    return consumed;
}

const Scene settings_scene_matter = {
    .enter_callback = settings_scene_matter_on_enter,
    .exit_callback = settings_scene_matter_on_exit,
    .event_callback = settings_scene_matter_on_event,
    .data_size = sizeof(SettingsSceneMatter),
};
