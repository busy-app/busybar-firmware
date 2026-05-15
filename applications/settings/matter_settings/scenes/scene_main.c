#include "../matter_settings_i.h"

#include <gui/modules/submenu.h>
#include <gui/modules/status_view.h>

typedef enum {
    SceneSubmenuIndexPairing,
    SceneSubmenuIndexReset,
} SceneSubmenuIndex;

typedef enum {
    SceneCustomEventMenuItemClicked = AppEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    Submenu* submenus[GuiDisplayIdMax];

    bool ui_initialized;
    _Atomic size_t menu_idx;
} MatterScene;

static void matter_scene_submenu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    MatterSettings* app = context;
    MatterScene* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdMain);

    scene->menu_idx = index;
    matter_settings_send_custom_event(app, SceneCustomEventMenuItemClicked);
}

static void matter_scene_on_enter(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    MatterScene* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdMain);

    scene->ui_initialized = false;

    MatterCommissionedFabrics fabrics;
    const MatterStatus status = matter_get_commissioned_fabrics(app->matter, &fabrics);

    if(status != MatterStatusOk) {
        scene_manager_next_scene(app->scene_manager, SceneIdWrecked);
        return;
    }

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* window = (display == GuiDisplayIdFront) ? app->front_scene_window :
                                                              app->back_scene_window;
            scene->submenus[display] = submenu_alloc(window);
            bool add_callback = display == GuiDisplayIdBack;

            submenu_add_item(
                scene->submenus[display],
                "Pair device",
                SceneSubmenuIndexPairing,
                add_callback ? matter_scene_submenu_item_callback : NULL,
                app);

            if(fabrics.count != 0) {
                submenu_add_item(
                    scene->submenus[display],
                    "Forget all pairings",
                    SceneSubmenuIndexReset,
                    add_callback ? matter_scene_submenu_item_callback : NULL,
                    app);
            }
        }
    });

    scene->ui_initialized = true;
}

static void matter_scene_on_exit(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    MatterScene* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdMain);

    if(!scene->ui_initialized) return;

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            submenu_free(scene->submenus[display]);
        }
    });
}

static bool matter_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    MatterSettings* app = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            MatterScene* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdMain);

            if(scene->menu_idx == SceneSubmenuIndexPairing) {
                scene_manager_next_scene(app->scene_manager, SceneIdPairing);
            } else if(scene->menu_idx == SceneSubmenuIndexReset) {
                matter_factory_reset(app->matter);
            } else {
                furi_crash();
            }

            consumed = true;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        matter_settings_exit_if_last(app);
    }

    return consumed;
}

const Scene matter_scene_main = {
    .enter_callback = matter_scene_on_enter,
    .exit_callback = matter_scene_on_exit,
    .event_callback = matter_scene_on_event,
    .data_size = sizeof(MatterScene),
};
