#include "../matter_settings.h"
#include <settings_helpers/status_view.h>

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} SettingsSceneCommissionFail;

static void matter_scene_commission_fail_on_enter(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    SettingsSceneCommissionFail* scene =
        scene_manager_get_scene_data(app->scene_manager, SceneIdCommissionFail);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = app->front_scene_window,
        [GuiDisplayIdBack] = app->back_scene_window,
    };

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("cross_front_7x7.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("cross_back_10x11.bin"),
    };

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Cannot connect");
        }
    });
}

static void matter_scene_commission_fail_on_exit(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    SettingsSceneCommissionFail* scene =
        scene_manager_get_scene_data(app->scene_manager, SceneIdCommissionFail);

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool matter_scene_commission_fail_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    MatterSettings* app = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        matter_settings_exit_if_last(app);
    }

    return consumed;
}

const Scene matter_scene_commission_fail = {
    .enter_callback = matter_scene_commission_fail_on_enter,
    .exit_callback = matter_scene_commission_fail_on_exit,
    .event_callback = matter_scene_commission_fail_on_event,
    .data_size = sizeof(SettingsSceneCommissionFail),
};
