#include "../matter_settings.h"
#include <settings_helpers/status_view.h>

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} MatterSceneCommissionDone;

static void matter_scene_commission_done_on_enter(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    MatterSceneCommissionDone* scene = scene_manager_get_current_scene_data(app->scene_manager);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = app->front_scene_window,
        [GuiDisplayIdBack] = app->back_scene_window,
    };

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("checkmark_front_8x6.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("checkmark_back_12x10.bin"),
    };

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Connected");
        }
    });
}

static void matter_scene_commission_done_on_exit(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    MatterSceneCommissionDone* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool matter_scene_commission_done_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    MatterSettings* app = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        matter_settings_exit_if_last(app);
    }

    return consumed;
}

const Scene matter_scene_commission_done = {
    .enter_callback = matter_scene_commission_done_on_enter,
    .exit_callback = matter_scene_commission_done_on_exit,
    .event_callback = matter_scene_commission_done_on_event,
    .data_size = sizeof(MatterSceneCommissionDone),
};
