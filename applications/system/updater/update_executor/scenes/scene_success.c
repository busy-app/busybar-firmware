#include "../update_executor_i.h"
#include "scenes.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>

typedef struct {
    FlexBox* front_box;

    FlexBox* back_box;
} UpdateExecutorSuccessScene;

static void update_executor_success_scene_on_enter(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorSuccessScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxSuccess);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_box = flex_box_alloc(instance->front_container);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        Image* front_image = image_alloc(flex_box_get_base(scene->front_box));
        image_set_source(front_image, SHARED_IMG_PATH("checkmark_front_8x8.image"));

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_text(front_label, "Update completed");
        label_set_font(front_label, FONT_BUSY_REGULAR_5);

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_container);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->back_box, 7);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        Image* back_image = image_alloc(flex_box_get_base(scene->back_box));
        image_set_source(back_image, SHARED_IMG_PATH("checkmark_back_11x11.image"));
        widget_set_padding(image_get_base(back_image), 2, 3, 2, 3);

        Label* back_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(back_label, "Update completed");
        label_set_font(back_label, FONT_BUSY_REGULAR_9);
    });
}

static void update_executor_success_scene_on_exit(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorSuccessScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxSuccess);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool update_executor_success_scene_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return true;
}

const Scene update_executor_internal_scene_success = {
    .enter_callback = update_executor_success_scene_on_enter,
    .exit_callback = update_executor_success_scene_on_exit,
    .event_callback = update_executor_success_scene_on_event,
    .data_size = sizeof(UpdateExecutorSuccessScene),
};
