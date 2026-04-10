#include "../update_executor_i.h"
#include "scenes.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>

#include <assets_images.h>

#define BACK_DETAIL_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88))

typedef struct {
    FlexBox* front_box;

    FlexBox* back_box;
    Label* back_detail_label;
} UpdateExecutorFailureScene;

static void update_executor_failure_scene_on_enter(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorFailureScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxFailure);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_box = flex_box_alloc(instance->front_container);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        Image* front_image = image_alloc(flex_box_get_base(scene->front_box));
        image_set_source(front_image, (void*)&I_error_front_8x8);

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_text(front_label, "Update failed");
        label_set_font(front_label, FONT_BUSY_REGULAR_5);

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_container);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        Image* back_image = image_alloc(flex_box_get_base(scene->back_box));
        image_set_source(back_image, (void*)&I_error_back_11x11);
        widget_set_padding(image_get_base(back_image), 2, 3, 2, 3);

        Label* back_primary_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(back_primary_label, "Update failed");
        label_set_font(back_primary_label, FONT_BUSY_REGULAR_9);
        widget_set_margin(label_get_base(back_primary_label), 0, 0, 6, 0);

        scene->back_detail_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(scene->back_detail_label, instance->update_status);
        label_set_font(scene->back_detail_label, FONT_BUSY_REGULAR_7);
        label_set_text_color(scene->back_detail_label, BACK_DETAIL_LABEL_TEXT_COLOR);
        widget_set_margin(label_get_base(scene->back_detail_label), 0, 0, 2, 0);
    });
}

static void update_executor_failure_scene_on_exit(void* context) {
    UpdateExecutor* instance = context;
    UpdateExecutorFailureScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxFailure);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool update_executor_failure_scene_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return true;
}

const Scene update_executor_internal_scene_failure = {
    .enter_callback = update_executor_failure_scene_on_enter,
    .exit_callback = update_executor_failure_scene_on_exit,
    .event_callback = update_executor_failure_scene_on_event,
    .data_size = sizeof(UpdateExecutorFailureScene),
};
