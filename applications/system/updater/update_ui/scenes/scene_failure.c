#include "../update_ui_i.h"

#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>

#define SCENE_EXIT_TIMEOUT_MS 4000

#define BACK_DETAIL_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88))

typedef struct {
    FlexBox* front_box;

    FlexBox* back_box;

    FuriEventLoopTimer* exit_timer;
} UpdateUiFailureScene;

static void update_ui_failure_scene_exit_timer_callback(void* context) {
    UpdateUi* instance = context;

    furi_event_loop_stop(instance->event_loop);
}

static void update_ui_failure_scene_on_enter(void* context) {
    furi_assert(context);

    UpdateUi* instance = context;
    UpdateUiFailureScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateUiSceneIdxFailure);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_box = flex_box_alloc(instance->front_scene_window);
        flex_box_set_flow(scene->front_box, FlexBoxFlowRow);
        flex_box_set_align(scene->front_box, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->front_box, 2);
        widget_set_align(flex_box_get_base(scene->front_box), AlignLeftMid);

        Image* front_image = image_alloc(flex_box_get_base(scene->front_box));
        image_set_source(front_image, SHARED_IMG_PATH("error_front_8x8.image"));

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_text(front_label, furi_string_get_cstr(instance->failure_preset.front_text));
        label_set_font(front_label, FONT_BUSY_REGULAR_5);

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_scene_window);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        Image* back_image = image_alloc(flex_box_get_base(scene->back_box));
        image_set_source(back_image, SHARED_IMG_PATH("error_back_11x11.image"));
        widget_set_padding(image_get_base(back_image), 2, 3, 2, 3);

        Label* back_primary_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(
            back_primary_label, furi_string_get_cstr(instance->failure_preset.back_primary_text));
        label_set_font(back_primary_label, FONT_BUSY_REGULAR_9);
        widget_set_margin(label_get_base(back_primary_label), 0, 0, 6, 0);

        Label* back_detail_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(
            back_detail_label, furi_string_get_cstr(instance->failure_preset.back_detail_text));
        label_set_font(back_detail_label, FONT_BUSY_REGULAR_7);
        label_set_text_color(back_detail_label, BACK_DETAIL_LABEL_TEXT_COLOR);
        widget_set_margin(label_get_base(back_detail_label), 0, 0, 2, 0);
    });

    scene->exit_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        update_ui_failure_scene_exit_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);

    furi_event_loop_timer_start(scene->exit_timer, furi_ms_to_ticks(SCENE_EXIT_TIMEOUT_MS));
}

static void update_ui_failure_scene_on_exit(void* context) {
    UpdateUi* instance = context;
    UpdateUiFailureScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateUiSceneIdxFailure);

    furi_event_loop_timer_free(scene->exit_timer);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool update_ui_failure_scene_on_event(const SceneManagerEvent* event, void* context) {
    UpdateUi* instance = context;

    if(event->type == SceneManagerEventTypeBack) {
        furi_event_loop_stop(instance->event_loop);
    }

    return true;
}

const Scene update_ui_internal_scene_failure = {
    .enter_callback = update_ui_failure_scene_on_enter,
    .exit_callback = update_ui_failure_scene_on_exit,
    .event_callback = update_ui_failure_scene_on_event,
    .data_size = sizeof(UpdateUiFailureScene),
};
