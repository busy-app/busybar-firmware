#include "../firmware_i.h"

#include <power/power_service/power.h>

#include <gui/modules/dialog.h>

typedef enum {
    ThisSceneEventInstall = ThisEventSceneEventsStart,
    ThisSceneEventCancel
} ThisSceneEvent;

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxDialog);
}

static void this_prepare_battery_low_result(ThisInstance* instance) {
    PowerInfo power_info;

    Power* power = furi_record_open(RECORD_POWER);
    power_get_info(power, &power_info);
    furi_record_close(RECORD_POWER);

    if(power_info.is_charging) {
        instance->result_preset.front_image_path = THIS_IMG_PATH("charging_battery_front_8x8.bin");
        furi_string_set(instance->result_preset.front_text, "Charging to 40%\nto start update...");

        furi_string_set(instance->result_preset.back_primary_text, "Battery is charging...");
    } else {
        instance->result_preset.front_image_path = THIS_IMG_PATH("low_battery_front_8x8.bin");
        furi_string_set(instance->result_preset.front_text, "Charge device up\nto 40% to update");

        furi_string_set(instance->result_preset.back_primary_text, "Charge your BUSY Bar");
    }

    instance->result_preset.back_image_path = SHARED_IMG_PATH("error_back_11x11.bin");
    furi_string_set(instance->result_preset.back_auxiliary_text, "40% needed to start update");

    instance->result_preset.timeout = FuriWaitForever;
}

static void this_dialog_option_callback(uint8_t result, void* context) {
    settings_firmware_app_fire_event(
        context, result ? ThisSceneEventCancel : ThisSceneEventInstall);
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    updater_get_check_info(instance->updater, &instance->update_info);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_dialog = dialog_alloc(instance->front_scene_window);
        dialog_set_callback(scene->front_dialog, this_dialog_option_callback, instance);
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

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    with_gui(instance->gui, {
        dialog_free(scene->back_dialog);
        dialog_free(scene->front_dialog);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisSceneEventInstall:
            UpdaterStatus session_status = updater_session_start(instance->updater);
            if(session_status == UpdaterStatusOk) {
                updater_install_from_url(
                    instance->updater,
                    furi_string_get_cstr(instance->update_info.url),
                    furi_string_get_cstr(instance->update_info.sha256));
            } else if(session_status == UpdaterStatusBatteryLow) {
                this_prepare_battery_low_result(instance);
                scene_manager_replace_current_scene(instance->scene_manager, ThisSceneIdxResult);
            }

            return true;

        case ThisSceneEventCancel:
            if(!scene_manager_search_and_switch_to_previous_scene(
                   instance->scene_manager, ThisSceneIdxMain)) {
                scene_manager_replace_current_scene(instance->scene_manager, ThisSceneIdxMain);
            }
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene settings_firmware_app_scene_dialog = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
