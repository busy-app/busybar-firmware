#include "../firmware_i.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/anim_player.h>

#include <toolbox/timers.h>

#define CHECK_TIMEOUT_MS (10 * 1000)

typedef enum {
    FirmwareSettingsCheckSceneEventAvailable = FirmwareSettingsEventSceneEventsStart,
    FirmwareSettingsCheckSceneEventNotAvailable,
    FirmwareSettingsCheckSceneEventFailure,
} FirmwareSettingsCheckSceneEvent;

typedef struct {
    FlexBox* front_box;
    FlexBox* back_box;

    FuriStateSub* check_subscription;
    CoarseTimer timeout_timer;
} FirmwareSettingsCheckScene;

static inline FirmwareSettingsCheckScene*
    firmware_settings_check_scene_get(FirmwareSettings* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxCheck);
}

static void firmware_settings_check_scene_prepare_up_to_date_result(FirmwareSettings* instance) {
    instance->check_result_preset.front_image_path = SHARED_IMG_PATH("checkmark_front_8x8.image");
    furi_string_set(instance->check_result_preset.front_text, "Up to date");

    instance->check_result_preset.back_image_path = SHARED_IMG_PATH("checkmark_back_11x11.image");
    furi_string_set(instance->check_result_preset.back_primary_text, "Firmware is up to date");
    furi_string_printf(
        instance->check_result_preset.back_detail_text,
        "Current version %s",
        updater_get_active_version());
}

static void firmware_settings_check_scene_prepare_failure_result(FirmwareSettings* instance) {
    instance->check_result_preset.front_image_path = SHARED_IMG_PATH("error_front_8x8.image");
    furi_string_set(instance->check_result_preset.front_text, "Unable to check");

    instance->check_result_preset.back_image_path = SHARED_IMG_PATH("error_back_11x11.image");
    furi_string_set(
        instance->check_result_preset.back_primary_text, "Unable to check\nfor update");
    furi_string_reset(instance->check_result_preset.back_detail_text);
}

static void firmware_settings_check_scene_update_check_callback(const void* item, void* context) {
    FirmwareSettings* instance = context;
    const UpdaterCheckState* _item = item;

    if(_item->event == UpdaterCheckEventStop) {
        FirmwareSettingsCheckSceneEvent event;

        switch(_item->result) {
        case UpdaterCheckResultAvailable:
            event = FirmwareSettingsCheckSceneEventAvailable;
            break;

        case UpdaterCheckResultNotAvailable:
            event = FirmwareSettingsCheckSceneEventNotAvailable;
            break;

        case UpdaterCheckResultFailure:
            event = FirmwareSettingsCheckSceneEventFailure;
            break;

        default:
            furi_crash();
        }

        firmware_settings_internal_fire_event(instance, event);
    }
}

static void firmware_settings_check_scene_on_enter(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsCheckScene* scene = firmware_settings_check_scene_get(instance);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_box = flex_box_alloc(instance->front_scene_window);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        AnimPlayer* front_anim = anim_player_alloc(flex_box_get_base(scene->front_box));
        anim_player_set_source(front_anim, SHARED_ANIM_PATH("spinner_front_8x8.anim"));

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_text(front_label, "Checking...");

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_scene_window);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->back_box, 8);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        AnimPlayer* back_anim = anim_player_alloc(flex_box_get_base(scene->back_box));
        anim_player_set_source(back_anim, SHARED_ANIM_PATH("spinner_back_16x16.anim"));

        Label* back_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(back_label, "Checking for update...");
    });

    FuriState* check_state = updater_get_check_state(instance->updater);
    scene->check_subscription = furi_state_subscribe(
        check_state, firmware_settings_check_scene_update_check_callback, instance);

    scene->timeout_timer = coarse_timer_create(CHECK_TIMEOUT_MS);
    updater_check_for_update(instance->updater);
}

static void firmware_settings_check_scene_on_exit(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsCheckScene* scene = firmware_settings_check_scene_get(instance);

    furi_state_unsubscribe(scene->check_subscription);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool firmware_settings_check_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsCheckScene* scene = firmware_settings_check_scene_get(instance);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case FirmwareSettingsCheckSceneEventAvailable:
            scene_manager_replace_current_scene(
                instance->scene_manager, FirmwareSettingsSceneIdxDialog);
            return true;

        case FirmwareSettingsCheckSceneEventNotAvailable:
            firmware_settings_check_scene_prepare_up_to_date_result(instance);
            scene_manager_replace_current_scene(
                instance->scene_manager, FirmwareSettingsSceneIdxCheckResult);
            return true;

        case FirmwareSettingsCheckSceneEventFailure:
            if(coarse_timer_is_expired(scene->timeout_timer)) {
                firmware_settings_check_scene_prepare_failure_result(instance);
                scene_manager_replace_current_scene(
                    instance->scene_manager, FirmwareSettingsSceneIdxCheckResult);
            } else {
                updater_check_for_update(instance->updater);
            }

            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene firmware_settings_internal_scene_check = {
    .enter_callback = firmware_settings_check_scene_on_enter,
    .exit_callback = firmware_settings_check_scene_on_exit,
    .event_callback = firmware_settings_check_scene_on_event,
    .data_size = sizeof(FirmwareSettingsCheckScene),
};
