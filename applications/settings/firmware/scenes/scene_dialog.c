#include "../firmware_i.h"

#include <power/power_service/power.h>

#include <gui/modules/dialog.h>

typedef enum {
    FirmwareSettingsDialogSceneEventInstall = FirmwareSettingsEventSceneEventsStart,
    FirmwareSettingsDialogSceneEventCancel
} FirmwareSettingsDialogSceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} FirmwareSettingsDialogScene;

static inline FirmwareSettingsDialogScene*
    firmware_settings_dialog_scene_get(FirmwareSettings* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxDialog);
}

static void firmware_settings_dialog_scene_option_callback(uint8_t result, void* context) {
    firmware_settings_internal_fire_event(
        context,
        result ? FirmwareSettingsDialogSceneEventCancel : FirmwareSettingsDialogSceneEventInstall);
}

static void firmware_settings_dialog_scene_on_enter(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsDialogScene* scene = firmware_settings_dialog_scene_get(instance);

    updater_get_check_info(instance->updater, &instance->update_info);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_dialog = dialog_alloc(instance->front_scene_window);
        dialog_set_callback(
            scene->front_dialog, firmware_settings_dialog_scene_option_callback, instance);
        dialog_set_icon(scene->front_dialog, THIS_IMG_PATH("download_front_8x8.image"));
        dialog_set_text(scene->front_dialog, "Update available");
        dialog_set_options(scene->front_dialog, "Install", "Cancel");
        dialog_set_option_colors(
            scene->front_dialog,
            (Color)COLOR_MAKE_RGB(0x22, 0xC5, 0x5E),
            (Color)COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF));

        /* back layout setup */
        scene->back_dialog = dialog_alloc(instance->back_scene_window);
        dialog_set_text(scene->back_dialog, "Update available");
        dialog_set_text_sub(
            scene->back_dialog, furi_string_get_cstr(instance->update_info.version));
        dialog_set_options(scene->back_dialog, "Install", "Cancel");
    });
}

static void firmware_settings_dialog_scene_on_exit(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsDialogScene* scene = firmware_settings_dialog_scene_get(instance);

    with_gui(instance->gui, {
        dialog_free(scene->front_dialog);
        dialog_free(scene->back_dialog);
    });
}

static bool
    firmware_settings_dialog_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case FirmwareSettingsDialogSceneEventInstall:
            UpdaterStatus session_status = updater_session_start(instance->updater);
            if(session_status == UpdaterStatusOk) {
                updater_install_from_url(
                    instance->updater,
                    furi_string_get_cstr(instance->update_info.url),
                    furi_string_get_cstr(instance->update_info.sha256));
            } else if(session_status == UpdaterStatusBatteryLow) {
                scene_manager_next_scene(
                    instance->scene_manager, FirmwareSettingsSceneIdxLowBattery);
            }
            return true;

        case FirmwareSettingsDialogSceneEventCancel:
            if(!scene_manager_search_and_switch_to_previous_scene(
                   instance->scene_manager, FirmwareSettingsSceneIdxMain)) {
                scene_manager_replace_current_scene(
                    instance->scene_manager, FirmwareSettingsSceneIdxMain);
            }
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene firmware_settings_internal_scene_dialog = {
    .enter_callback = firmware_settings_dialog_scene_on_enter,
    .exit_callback = firmware_settings_dialog_scene_on_exit,
    .event_callback = firmware_settings_dialog_scene_on_event,
    .data_size = sizeof(FirmwareSettingsDialogScene),
};
