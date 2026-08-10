#include "../update_executor_i.h"
#include "scenes.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/anim_player.h>
#include <gui/modules/label.h>

typedef struct {
    FlexBox* front_box;

    FlexBox* back_box;
} UpdateExecutorRebootScene;

static void update_executor_reboot_scene_on_enter(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorRebootScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxReboot);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_box = flex_box_alloc(instance->front_container);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        AnimPlayer* front_anim = anim_player_alloc(flex_box_get_base(scene->front_box));
        anim_player_set_source(front_anim, SHARED_ANIM_PATH("spinner_front_8x8.anim"));

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_text(front_label, "Restarting device");
        label_set_font(front_label, FONT_BUSY_REGULAR_5);

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_container);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->back_box, 7);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        AnimPlayer* back_anim = anim_player_alloc(flex_box_get_base(scene->back_box));
        anim_player_set_source(back_anim, SHARED_ANIM_PATH("spinner_back_16x16.anim"));

        Label* back_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(back_label, "Restarting device");
        label_set_font(back_label, FONT_BUSY_REGULAR_9);
    });
}

static void update_executor_reboot_scene_on_exit(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorRebootScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxReboot);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool update_executor_reboot_scene_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return true;
}

const Scene update_executor_internal_scene_reboot = {
    .enter_callback = update_executor_reboot_scene_on_enter,
    .exit_callback = update_executor_reboot_scene_on_exit,
    .event_callback = update_executor_reboot_scene_on_event,
    .data_size = sizeof(UpdateExecutorRebootScene),
};
