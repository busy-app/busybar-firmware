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

static inline ThisScene* get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxDialog);
}

static void dialog_option_callback(uint8_t result, void* context) {
    settings_firmware_app_fire_event(
        context, result ? ThisSceneEventCancel : ThisSceneEventInstall);
}

static void scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = get_scene(instance);

    updater_get_check_info(instance->updater, &instance->update_info);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_dialog = dialog_alloc(instance->front_scene_window);
        dialog_set_callback(scene->front_dialog, dialog_option_callback, instance);
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

static void scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = get_scene(instance);

    with_gui(instance->gui, {
        dialog_free(scene->back_dialog);
        dialog_free(scene->front_dialog);
    });
}

static bool scene_on_event(const SceneManagerEvent* event, void* context) {
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
                scene_manager_next_scene(instance->scene_manager, ThisSceneIdxLowBattery);
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

const Scene settings_firmware_internal_scene_dialog = {
    .enter_callback = scene_on_enter,
    .exit_callback = scene_on_exit,
    .event_callback = scene_on_event,
    .data_size = sizeof(ThisScene),
};
