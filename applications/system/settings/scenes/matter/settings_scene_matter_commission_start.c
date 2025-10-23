#include "../../settings.h"
#include "../../storage_macros.h"
#include "matter_scenes_common.h"
#include "../../widgets/status_view.h"

#include <matter/matter.h>

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} SettingsSceneCommissionStart;

static void settings_scene_matter_commission_start_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneCommissionStart* scene = scene_manager_get_current_scene_data(app->scene_manager);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = app->front_scene_window,
        [GuiDisplayIdBack] = app->back_scene_window,
    };

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("spinner_front_7x7.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("spinner_back_16x16.bin"),
    };

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Connecting...");
        }

        status_view_set_additional_text(
            scene->statuses[GuiDisplayIdBack], "Might take a few minutes");
    });
}

static void settings_scene_matter_commission_start_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneCommissionStart* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool
    settings_scene_matter_commission_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* app = context;
    UNUSED(app);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        consumed = matter_scene_replace_current(app, event->event);
    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene settings_scene_matter_commission_start = {
    .enter_callback = settings_scene_matter_commission_start_on_enter,
    .exit_callback = settings_scene_matter_commission_start_on_exit,
    .event_callback = settings_scene_matter_commission_start_on_event,
    .data_size = sizeof(SettingsSceneCommissionStart),
};
