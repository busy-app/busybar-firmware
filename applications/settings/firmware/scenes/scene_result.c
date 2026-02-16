#include "../firmware_i.h"

#include <gui/storage_macros.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>

typedef struct {
    FlexBox* front_box;
    FlexBox* back_box;

    FuriEventLoopTimer* timer;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxResult);
}

static void this_timer_callback(void* context) {
    ThisInstance* instance = context;
    scene_manager_previous_scene(instance->scene_manager);
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

        Image* front_image = image_alloc(flex_box_get_base(scene->front_box));
        image_set_source(front_image, instance->result_preset.front_image_path);

        Label* front_label = label_alloc(flex_box_get_base(scene->front_box));
        label_set_line_spacing(front_label, 0);
        label_set_text(front_label, furi_string_get_cstr(instance->result_preset.front_text));

        /* back layout setup */
        scene->back_box = flex_box_alloc(instance->back_scene_window);
        flex_box_set_flow(scene->back_box, FlexBoxFlowColumn);
        flex_box_set_align(scene->back_box, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(scene->back_box, 3);
        widget_set_align(flex_box_get_base(scene->back_box), AlignCenter);

        Image* back_image = image_alloc(flex_box_get_base(scene->back_box));
        image_set_source(back_image, instance->result_preset.back_image_path);
        widget_set_padding(image_get_base(back_image), 0, 0, 2, 7);

        Label* back_primary_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text(
            back_primary_label, furi_string_get_cstr(instance->result_preset.back_primary_text));

        Label* back_auxiliary_label = label_alloc(flex_box_get_base(scene->back_box));
        label_set_text_color(back_auxiliary_label, (Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88));
        label_set_text(
            back_auxiliary_label,
            furi_string_get_cstr(instance->result_preset.back_auxiliary_text));
    });

    if(instance->result_preset.timeout != FuriWaitForever) {
        scene->timer = furi_event_loop_timer_alloc(
            instance->event_loop, this_timer_callback, FuriEventLoopTimerTypeOnce, instance);
        furi_event_loop_timer_start(scene->timer, instance->result_preset.timeout);
    }
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    if(scene->timer) furi_event_loop_timer_free(scene->timer);

    with_gui(instance->gui, {
        flex_box_free(scene->back_box);
        flex_box_free(scene->front_box);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return false;
}

const Scene settings_firmware_app_scene_result = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
