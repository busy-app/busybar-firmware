#include "../system_settings.h"
#include <settings_helpers/status_view.h>
#include "../settings_helpers/gui_params.h"

#include <power/power_service/power.h>

#define REBOOT_TIMER_MS (2500)

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} SettingsSceneReboot;

static void system_settings_scene_power_restart_on_enter(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SettingsSceneReboot* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerRestart);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("spinner_front_7x7.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("spinner_back_16x16.bin"),
    };

    with_gui(instance->gui, {
        nav_bar_push_location(instance->back_nav_bar, "RESTART DEVICE");

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Restarting device...");
        }
    });

    furi_delay_ms(REBOOT_TIMER_MS);
    // TODO Make factory reset

    power_reboot(instance->power, PowerRebootNormal);
    while(1)
        ;
}

static void system_settings_scene_power_restart_on_exit(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SettingsSceneReboot* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerRestart);

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool
    system_settings_scene_power_restart_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    UNUSED(instance);
    UNUSED(event);

    bool consumed = false;
    return consumed;
}

const Scene system_settings_scene_power_restart = {
    .enter_callback = system_settings_scene_power_restart_on_enter,
    .exit_callback = system_settings_scene_power_restart_on_exit,
    .event_callback = system_settings_scene_power_restart_on_event,
    .data_size = sizeof(SettingsSceneReboot),
};
