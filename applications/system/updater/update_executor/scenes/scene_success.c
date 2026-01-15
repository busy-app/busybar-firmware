#include "../update_executor_i.h"
#include "../../storage_macros.h"
#include "scenes.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/anim_play.h>
#include <gui/modules/label.h>
#include <gui/storage_macros.h>

#define SUCCESS_SCENE_DELAY_MS 1500

typedef struct {
    FlexLayout* back_flex;
    FlexLayout* front_flex;
} SuccessScene;

static void success_scene_on_enter(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    SuccessScene* data =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdSuccess);

    with_gui(instance->gui, {
        /* back ui */
        data->back_flex = flex_layout_alloc(instance->back_container, FlexLayoutTypeColumn);
        flex_layout_set_spacing(data->back_flex, 6);
        flex_layout_set_align(
            data->back_flex, FlexLayoutAlignCenter, FlexLayoutAlignCenter, FlexLayoutAlignStart);
        widget_set_size_content(flex_layout_get_base(data->back_flex));
        widget_set_align(flex_layout_get_base(data->back_flex), AlignCenter);

        AnimPlay* back_spinner_anim_play = anim_play_alloc(flex_layout_get_base(data->back_flex));
        if(anim_play_set_source(back_spinner_anim_play, GUI_ANIM_PATH("spinner_back_16x16.anim"))) {
            anim_play_loop_whole(back_spinner_anim_play);
        }
        widget_set_size_content(anim_play_get_base(back_spinner_anim_play));

        Label* back_status_label = label_alloc(flex_layout_get_base(data->back_flex));
        label_set_text_font_size(back_status_label, LabelFontSizeLarge);
        label_set_text(back_status_label, "Restarting device...");
        widget_set_size_content(label_get_base(back_status_label));

        /* front ui */
        data->front_flex = flex_layout_alloc(instance->front_container, FlexLayoutTypeRow);
        flex_layout_set_spacing(data->front_flex, 2);
        widget_set_size_content(flex_layout_get_base(data->front_flex));
        widget_set_align(flex_layout_get_base(data->front_flex), AlignLeftMid);

        AnimPlay* front_spinner_anim_play =
            anim_play_alloc(flex_layout_get_base(data->front_flex));
        if(anim_play_set_source(front_spinner_anim_play, GUI_ANIM_PATH("spinner_front_8x8.anim"))) {
            anim_play_loop_whole(front_spinner_anim_play);
        }
        widget_set_size_content(anim_play_get_base(front_spinner_anim_play));

        Label* front_status_label = label_alloc(flex_layout_get_base(data->front_flex));
        label_set_text(front_status_label, "Restarting device...");
        widget_set_size_content(label_get_base(front_status_label));
    });
}

static void success_scene_on_exit(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    SuccessScene* data =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdSuccess);

    with_gui(instance->gui, {
        flex_layout_free(data->back_flex);
        flex_layout_free(data->front_flex);
    });
}

static bool success_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event->type == SceneManagerEventTypeCustom);

    furi_delay_ms(SUCCESS_SCENE_DELAY_MS);

    return true;
}

const Scene update_executor_scene_success = {
    .enter_callback = success_scene_on_enter,
    .exit_callback = success_scene_on_exit,
    .event_callback = success_scene_on_event,
    .data_size = sizeof(SuccessScene),
};
