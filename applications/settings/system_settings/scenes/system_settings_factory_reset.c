#include "../system_settings.h"
#include "../settings_helpers/gui_params.h"

#include <settings_helpers/status_view.h>
#include <toolbox/update_lib/factory_reset.h>

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

    static const char* const images[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = SETTINGS_IMG_PATH("spinner_front_7x7.bin"),
        [GuiDisplayIdBack] = SETTINGS_IMG_PATH("spinner_back_16x16.bin"),
    };

    with_gui(instance->gui, {
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            scene->statuses[disp] = status_view_alloc(windows[disp]);
            status_view_set_icon(scene->statuses[disp], images[disp]);
            status_view_set_header(scene->statuses[disp], "Restarting device...");
        }
    });

    Updater* updater = furi_record_open(RECORD_UPDATER);
    UpdaterStatus update_status = updater_session_start(updater);
    while(update_status != UpdaterStatusOk) {
        furi_delay_ms(5000);
        update_status = updater_session_start(updater);
    }
    factory_reset_perform(updater, false);

    updater_session_stop(updater);
    furi_record_close(RECORD_UPDATER);

    while(true)
        ;
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
    furi_assert(context);

    SystemSettings* instance = context;
    UNUSED(instance);
    UNUSED(event);

    bool consumed = false;
    return consumed;
}

const Scene system_settings_scene_factory_reset = {
    .enter_callback = system_settings_scene_factory_reset_on_enter,
    .exit_callback = system_settings_scene_factory_reset_on_exit,
    .event_callback = system_settings_scene_factory_reset_on_event,
    .data_size = sizeof(SettingsSceneReboot),
};
