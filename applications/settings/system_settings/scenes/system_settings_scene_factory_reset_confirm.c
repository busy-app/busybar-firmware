#include "../system_settings.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/dialog.h>

typedef enum {
    SceneEventConfirm = AppEventSceneEventsStart,
    SceneEventCancel,
} SceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} SceneSystemFactoryResetConfirm;

static void system_settings_scene_factory_reset_confirm_callback(uint8_t result, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    if(result == 0) {
        system_settings_send_custom_event(instance, SceneEventConfirm);
    } else {
        system_settings_send_custom_event(instance, SceneEventCancel);
    }
}

static void system_settings_scene_factory_reset_confirm_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SceneSystemFactoryResetConfirm* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFactoryResetConfirm);

    with_gui(instance->gui, {
        /* front layout setup */
        data->front_dialog = dialog_alloc(instance->front_scene_window);
        dialog_set_icon(data->front_dialog, SHARED_IMG_PATH("error_front_8x8.image"));
        dialog_set_text(data->front_dialog, "Reset\ndevice?");
        dialog_set_option_colors(
            data->front_dialog,
            (Color)COLOR_MAKE_RGB(0xED, 0x00, 0x18),
            (Color)COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF));
        dialog_set_options(data->front_dialog, "Reset", "Cancel");
        dialog_select_option(data->front_dialog, 1);
        dialog_set_callback(
            data->front_dialog, system_settings_scene_factory_reset_confirm_callback, instance);

        data->back_dialog = dialog_alloc(instance->back_scene_window);
        dialog_set_text(data->back_dialog, "Reset the device? All user\ndata will be erased");
        dialog_set_options(data->back_dialog, "Reset", "Cancel");
        dialog_select_option(data->back_dialog, 1);
    });
}

static void system_settings_scene_factory_reset_confirm_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SceneSystemFactoryResetConfirm* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFactoryResetConfirm);

    with_gui(instance->gui, {
        dialog_free(data->front_dialog);
        dialog_free(data->back_dialog);
    });
}

static bool system_settings_scene_factory_reset_confirm_on_event(
    const SceneManagerEvent* event,
    void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventConfirm) {
            SceneId scene_id =
                (updater_get_allowance_status(instance->updater) != UpdaterStatusBatteryLow) ?
                    SceneIdFactoryReset :
                    SceneIdLowBattery;
            scene_manager_next_scene(instance->scene_manager, scene_id);
            consumed = true;
        } else if(event->event == SceneEventCancel) {
            system_settings_pop_location(instance);
            scene_manager_previous_scene(instance->scene_manager);
            consumed = true;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        system_settings_pop_location(instance);
        scene_manager_previous_scene(instance->scene_manager);
        consumed = true;
    }

    return consumed;
}

const Scene system_settings_scene_factory_reset_confirm = {
    .enter_callback = system_settings_scene_factory_reset_confirm_on_enter,
    .exit_callback = system_settings_scene_factory_reset_confirm_on_exit,
    .event_callback = system_settings_scene_factory_reset_confirm_on_event,
    .data_size = sizeof(SceneSystemFactoryResetConfirm),
};
