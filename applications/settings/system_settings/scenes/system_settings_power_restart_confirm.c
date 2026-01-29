#include "../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/dialog.h>

#include <power/power_service/power.h>

typedef enum {
    SceneEventConfirm = AppEventSceneEventsStart,
    SceneEventCancel,
} SceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} SceneSystemFactoryResetConfirm;

static void system_settings_scene_power_restart_confirm_callback(uint8_t result, void* context) {
    SystemSettings* instance = context;
    if(result == 0) {
        system_settings_send_custom_event(instance, SceneEventConfirm);
    } else {
        system_settings_send_custom_event(instance, SceneEventCancel);
    }
}

static void system_settings_scene_power_scene_restart_confirm_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SceneSystemFactoryResetConfirm* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerRestartConfirm);

    with_gui(instance->gui, {
        data->front_dialog = dialog_alloc(instance->front_scene_window);
        data->back_dialog = dialog_alloc(instance->back_scene_window);

        dialog_set_text(data->front_dialog, "Restart device?");
        dialog_set_text(data->back_dialog, "Restart device?");

        Color text_color = COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF);
        dialog_set_option_colors(data->front_dialog, text_color, text_color);

        dialog_set_options(data->front_dialog, "Restart", "Cancel");
        dialog_set_options(data->back_dialog, "Restart", "Cancel");

        dialog_set_calback(
            data->front_dialog, system_settings_scene_power_restart_confirm_callback, instance);
    });
}

static void system_settings_scene_power_scene_restart_confirm_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SceneSystemFactoryResetConfirm* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerRestartConfirm);

    with_gui(instance->gui, {
        dialog_free(data->front_dialog);
        dialog_free(data->back_dialog);
    });
}

static bool system_settings_scene_power_scene_restart_confirm_on_event(
    const SceneManagerEvent* event,
    void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventConfirm) {
            scene_manager_next_scene(instance->scene_manager, SceneIdPowerRestart);
            consumed = true;
        } else if(event->event == SceneEventCancel) {
            system_settings_pop_location(instance);
            consumed = scene_manager_previous_scene(instance->scene_manager);
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        system_settings_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene system_settings_scene_power_restart_confirm = {
    .enter_callback = system_settings_scene_power_scene_restart_confirm_on_enter,
    .exit_callback = system_settings_scene_power_scene_restart_confirm_on_exit,
    .event_callback = system_settings_scene_power_scene_restart_confirm_on_event,
    .data_size = sizeof(SceneSystemFactoryResetConfirm),
};
