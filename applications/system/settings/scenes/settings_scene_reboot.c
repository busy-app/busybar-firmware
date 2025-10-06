#include "../storage_macros.h"
#include "../settings.h"
#include "settings_scenes.h"
#include "../widgets/status_view.h"

#include <matter/matter.h>
#include <power/power_service/power.h>

#define REBOOT_TIMER_MS (2500)

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} SettingsSceneReboot;

static void settings_scene_reboot_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneReboot* scene = scene_manager_get_current_scene_data(app->scene_manager);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = app->front_scene_window,
        [GuiDisplayIdBack] = app->back_scene_window,
    };

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("spinner_front_7x7.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("spinner_back_16x16.bin"),
    };

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Restarting device...");
        }
    });

    furi_delay_ms(REBOOT_TIMER_MS);
    Power* power = furi_record_open(RECORD_POWER);
    power_reboot(power, PowerRebootNormal);
    while(1)
        ;
}

static void settings_scene_reboot_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneReboot* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool settings_scene_reboot_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* app = context;
    UNUSED(app);
    UNUSED(event);

    bool consumed = false;
    return consumed;
}

const Scene settings_scene_reboot = {
    .enter_callback = settings_scene_reboot_on_enter,
    .exit_callback = settings_scene_reboot_on_exit,
    .event_callback = settings_scene_reboot_on_event,
    .data_size = sizeof(SettingsSceneReboot),
};
