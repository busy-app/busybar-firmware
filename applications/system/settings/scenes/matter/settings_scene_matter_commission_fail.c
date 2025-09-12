#include "../../settings.h"

#include <gui/modules/label.h>

#include <matter/matter.h>

typedef struct {
    Label* labels[GuiDisplayIdMax];
} SettingsSceneDebugApps;

static void settings_scene_matter_commission_fail_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* window = (display == GuiDisplayIdFront) ? app->front_scene_window :
                                                              app->back_scene_window;
            scene->labels[display] = label_alloc(window);
            label_set_text(scene->labels[display], "Commissioning error");
        }
    });
}

static void settings_scene_matter_commission_fail_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            label_free(scene->labels[display]);
        }
    });
}

static bool
    settings_scene_matter_commission_fail_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    furi_assert(context);

    SettingsApp* app = context;
    UNUSED(app);

    bool consumed = false;

    return consumed;
}

const Scene settings_scene_matter_commission_fail = {
    .enter_callback = settings_scene_matter_commission_fail_on_enter,
    .exit_callback = settings_scene_matter_commission_fail_on_exit,
    .event_callback = settings_scene_matter_commission_fail_on_event,
    .data_size = sizeof(SettingsSceneDebugApps),
};
