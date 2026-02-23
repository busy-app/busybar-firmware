#include "../firmware_i.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/anim_player.h>

typedef enum {
    ThisSceneEventAvailable = ThisEventSceneEventsStart,
    ThisSceneEventNotAvailable
} ThisSceneEvent;

typedef struct {
    FlexBox* front_box;
    FlexBox* back_box;

    FuriStateSub* check_subscription;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxCheck);
}

static void this_prepare_up_to_date_result(ThisInstance* instance) {
    instance->result_preset.front_image_path = THIS_IMG_PATH("checkmark_front_8x8.bin");
    furi_string_set(instance->result_preset.front_text, "Up to date");

    instance->result_preset.back_image_path = THIS_IMG_PATH("checkmark_back_11x11.bin");
    furi_string_set(instance->result_preset.back_primary_text, "Firmware is up to date");
    furi_string_printf(
        instance->result_preset.back_auxiliary_text,
        "Current version %s",
        updater_get_active_version());

    instance->result_preset.timeout = 3000;
}

static void this_check_callback(const void* item, void* context) {
    ThisInstance* instance = context;
    const UpdaterCheckState* _item = item;

    if(_item->event == UpdaterCheckEventStop) {
        settings_firmware_app_fire_event(
            instance,
            (_item->result == UpdaterCheckResultAvailable) ? ThisSceneEventAvailable :
                                                             ThisSceneEventNotAvailable);
    }
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

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
    scene->check_subscription = furi_state_subscribe(check_state, this_check_callback, instance);

    updater_check_for_update(instance->updater);
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    furi_state_unsubscribe(scene->check_subscription);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisSceneEventAvailable:
            scene_manager_replace_current_scene(instance->scene_manager, ThisSceneIdxDialog);
            return true;

        case ThisSceneEventNotAvailable:
            this_prepare_up_to_date_result(instance);
            scene_manager_replace_current_scene(instance->scene_manager, ThisSceneIdxResult);
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene settings_firmware_app_scene_check = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
