#include "../matter_settings_i.h"

#include <gui/modules/status_view.h>

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} SettingsSceneWrecked;

static void matter_scene_wrecked_on_enter(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    SettingsSceneWrecked* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdWrecked);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = app->front_scene_window,
        [GuiDisplayIdBack] = app->back_scene_window,
    };

    static const struct {
        const char* image;
        const char* text;
    } displays[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] =
            {
                .image = SHARED_IMG_PATH("error_front_8x8.image"),
                .text = "Contact support",
            },
        [GuiDisplayIdBack] =
            {
                .image = SHARED_IMG_PATH("error_back_11x11.image"),
                .text = "Smart home integration is\nbroken on this BUSY Bar.\nContact support.",
            },
    };

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], displays[disp].image, false);
            status_view_set_primary_text(scene->statuses[disp], displays[disp].text);
        }
    });
}

static void matter_scene_wrecked_on_exit(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    SettingsSceneWrecked* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdWrecked);

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool matter_scene_wrecked_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    MatterSettings* app = context;
    UNUSED(app);
    UNUSED(event);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        matter_settings_exit_if_last(app);
    }

    return consumed;
}

const Scene matter_scene_wrecked = {
    .enter_callback = matter_scene_wrecked_on_enter,
    .exit_callback = matter_scene_wrecked_on_exit,
    .event_callback = matter_scene_wrecked_on_event,
    .data_size = sizeof(SettingsSceneWrecked),
};
