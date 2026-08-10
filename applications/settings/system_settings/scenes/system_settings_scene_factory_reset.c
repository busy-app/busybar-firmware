#include "../system_settings.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/status_view.h>

#include <toolbox/update_lib/factory_reset.h>

#define REBOOT_TIMER_MS       (2500)
#define WAIT_UPDATE_UNLOCK_MS (5000)

typedef struct {
    StatusView* statuses[GuiDisplayIdMax];
} SettingsSceneReboot;

static void system_settings_scene_factory_reset_on_enter(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SettingsSceneReboot* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFactoryReset);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    static const char* const anims[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SHARED_ANIM_PATH("spinner_front_8x8.anim"),
        [GuiDisplayIdBack] = SHARED_ANIM_PATH("spinner_back_16x16.anim"),
    };

    with_gui(instance->gui, {
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], anims[disp], true);
            status_view_set_primary_text(scene->statuses[disp], "Restarting device");
        }
    });

    UpdaterStatus update_status = updater_session_start(instance->updater);
    while(update_status == UpdaterStatusBusy) {
        furi_delay_ms(WAIT_UPDATE_UNLOCK_MS);
        update_status = updater_session_start(instance->updater);
    }

    if(update_status == UpdaterStatusOk) {
        furi_delay_ms(REBOOT_TIMER_MS);
        factory_reset_perform(instance->updater, false);
        updater_session_stop(instance->updater);

        while(true)
            ;
    } else if(update_status == UpdaterStatusBatteryLow) {
        scene_manager_replace_current_scene(instance->scene_manager, SceneIdLowBattery);
    }
}

static void system_settings_scene_factory_reset_on_exit(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SettingsSceneReboot* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFactoryReset);

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            status_view_free(scene->statuses[disp]);
        }
    });
}

static bool
    system_settings_scene_factory_reset_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

const Scene system_settings_scene_factory_reset = {
    .enter_callback = system_settings_scene_factory_reset_on_enter,
    .exit_callback = system_settings_scene_factory_reset_on_exit,
    .event_callback = system_settings_scene_factory_reset_on_event,
    .data_size = sizeof(SettingsSceneReboot),
};
