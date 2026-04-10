#include "../update_executor_i.h"
#include "scenes.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/progress_bar.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>
#include <gui/modules/overlap_fader.h>

#include <assets_images.h>

#define BACK_DETAIL_LABEL_TEXT_COLOR ((Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88))

typedef struct {
    FlexLayout* front_layout;
    Label* front_percent_label;
    OverlapFader* front_fader;
    ProgressBar* front_progress_bar;

    FlexLayout* back_layout;
    Label* back_percent_label;
    ProgressBar* back_progress_bar;
    Label* back_detail_label;
} UpdateExecutorInstallScene;

static void update_executor_install_scene_on_enter(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorInstallScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxInstall);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_layout = flex_layout_alloc(instance->front_container, FlexLayoutTypeColumn);
        flex_layout_set_spacing(scene->front_layout, 1);

        FlexLayout* front_status_layout =
            flex_layout_alloc(flex_layout_get_base(scene->front_layout), FlexLayoutTypeRow);
        widget_set_height(flex_layout_get_base(front_status_layout), 9); /* font's line height */

        Label* front_status_label = label_alloc(flex_layout_get_base(front_status_layout));
        label_set_text(front_status_label, "Installing update");
        label_set_font(front_status_label, FONT_BUSY_REGULAR_5);
        widget_set_size_content(label_get_base(front_status_label));
        flex_layout_set_child_widget_grow(
            front_status_layout, label_get_base(front_status_label), 1);

        scene->front_percent_label = label_alloc(flex_layout_get_base(front_status_layout));
        label_set_text(scene->front_percent_label, "0%");
        label_set_font(scene->front_percent_label, FONT_BUSY_REGULAR_5);
        widget_set_size_content(label_get_base(scene->front_percent_label));
        flex_layout_set_child_widget_grow(
            front_status_layout, label_get_base(scene->front_percent_label), 0);

        scene->front_fader = overlap_fader_alloc(flex_layout_get_base(front_status_layout));
        overlap_fader_align_to(
            scene->front_fader, label_get_base(scene->front_percent_label), OverlapFaderSideLeft);
        widget_set_width(overlap_fader_get_base(scene->front_fader), 10);

        scene->front_progress_bar = progress_bar_alloc(flex_layout_get_base(scene->front_layout));
        widget_set_height(progress_bar_get_base(scene->front_progress_bar), 4);

        /* back layout setup */
        scene->back_layout = flex_layout_alloc(instance->back_container, FlexLayoutTypeColumn);
        flex_layout_set_align(
            scene->back_layout, FlexLayoutAlignEnd, FlexLayoutAlignStart, FlexLayoutAlignCenter);
        widget_set_padding(flex_layout_get_base(scene->back_layout), 2, 2, 0, 0);

        FlexBox* back_status_container = flex_box_alloc(flex_layout_get_base(scene->back_layout));
        flex_box_set_flow(back_status_container, FlexBoxFlowRow);
        flex_box_set_align(back_status_container, FlexBoxAlignStart, FlexBoxAlignEnd);
        flex_box_set_spacing(back_status_container, 4);
        widget_set_margin(flex_box_get_base(back_status_container), 0, 0, 0, 7);

        Image* back_status_image = image_alloc(flex_box_get_base(back_status_container));
        image_set_source(back_status_image, (void*)&I_install_back_12x12);
        widget_set_padding(image_get_base(back_status_image), 0, 0, 0, 1);

        Label* back_status_label = label_alloc(flex_box_get_base(back_status_container));
        label_set_text(back_status_label, "Installing update");
        label_set_font(back_status_label, FONT_BUSY_REGULAR_9);

        scene->back_percent_label = label_alloc(flex_box_get_base(back_status_container));
        label_set_text(scene->back_percent_label, "0%");
        label_set_font(scene->back_percent_label, FONT_BUSY_REGULAR_9);
        widget_set_ignore_layout(label_get_base(scene->back_percent_label), true);
        widget_set_align(label_get_base(scene->back_percent_label), AlignRightMid);

        scene->back_progress_bar = progress_bar_alloc(flex_layout_get_base(scene->back_layout));
        widget_set_height(progress_bar_get_base(scene->back_progress_bar), 6);
        widget_set_margin(progress_bar_get_base(scene->back_progress_bar), 0, 0, 0, 15);

        FlexBox* back_detail_container = flex_box_alloc(flex_layout_get_base(scene->back_layout));
        flex_box_set_flow(back_detail_container, FlexBoxFlowRow);
        flex_box_set_align(back_detail_container, FlexBoxAlignCenter, FlexBoxAlignCenter);
        flex_box_set_spacing(back_detail_container, 3);
        widget_set_padding(flex_box_get_base(back_detail_container), 0, 0, 0, 8);

        scene->back_detail_label = label_alloc(flex_box_get_base(back_detail_container));
        label_set_text(scene->back_detail_label, "");
        label_set_font(scene->back_detail_label, FONT_BUSY_REGULAR_7);
        label_set_text_color(scene->back_detail_label, BACK_DETAIL_LABEL_TEXT_COLOR);
    });
}

static void update_executor_install_scene_on_exit(void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorInstallScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxInstall);

    with_gui(instance->gui, {
        flex_layout_free(scene->back_layout);
        flex_layout_free(scene->front_layout);
    });
}

static bool update_executor_install_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    UpdateExecutor* instance = context;
    UpdateExecutorInstallScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, UpdateExecutorSceneIdxInstall);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case UpdateExecutorEventUpdateProgress:
            with_gui(instance->gui, {
                label_set_text_fmt(scene->front_percent_label, "%u%%", instance->update_percent);
                progress_bar_set_value(scene->front_progress_bar, instance->update_percent);

                label_set_text_fmt(scene->back_percent_label, "%u%%", instance->update_percent);
                progress_bar_set_value(scene->back_progress_bar, instance->update_percent);
                label_set_text(scene->back_detail_label, instance->update_status);

                overlap_fader_realign(scene->front_fader);
            });
            break;

        default:
            break;
        }
    }

    return true;
}

const Scene update_executor_internal_scene_install = {
    .enter_callback = update_executor_install_scene_on_enter,
    .exit_callback = update_executor_install_scene_on_exit,
    .event_callback = update_executor_install_scene_on_event,
    .data_size = sizeof(UpdateExecutorInstallScene),
};
