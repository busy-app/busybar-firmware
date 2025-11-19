#include "../update_app_i.h"
#include "../storage_macros.h"
#include "updater_scenes.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>
#include <gui/modules/progress_bar.h>

#define BACK_EXTRAS_LABEL_COLOR 0x777777

typedef struct {
    FlexLayout* back_flex;
    Label* back_status_percent_label;
    Label* back_extras_label;
    ProgressBar* back_progress_bar;

    FlexLayout* front_flex;
    Label* front_status_percent_label;
    ProgressBar* front_progress_bar;
} UpdaterSceneInstall;

static void updater_scene_install_on_enter(void* context) {
    furi_assert(context);

    UpdaterApp* instance = context;
    UpdaterSceneInstall* data =
        scene_manager_get_scene_data(instance->scene_manager, UpdaterAppSceneIdInstall);

    with_gui(instance->gui, {
        /* back ui */
        data->back_flex = flex_layout_alloc(instance->back_container, FlexLayoutTypeColumn);
        flex_layout_set_spacing(data->back_flex, 6);
        flex_layout_set_align(
            data->back_flex, FlexLayoutAlignCenter, FlexLayoutAlignStart, FlexLayoutAlignStart);
        widget_set_padding(flex_layout_get_base(data->back_flex), 4, 4, 0, 0);

        FlexLayout* back_status_line_flex =
            flex_layout_alloc(flex_layout_get_base(data->back_flex), FlexLayoutTypeRow);
        flex_layout_set_spacing(back_status_line_flex, 5);
        flex_layout_set_align(
            back_status_line_flex,
            FlexLayoutAlignStart,
            FlexLayoutAlignCenter,
            FlexLayoutAlignStart);
        widget_set_height_content(flex_layout_get_base(back_status_line_flex));

        Image* back_status_image = image_alloc(flex_layout_get_base(back_status_line_flex));
        image_set_source(back_status_image, UPDATER_IMG_PATH("install_back_12x12.bin"));
        widget_set_size_content(image_get_base(back_status_image));

        Label* back_status_label = label_alloc(flex_layout_get_base(back_status_line_flex));
        label_set_text(back_status_label, "Installing");
        widget_set_size_content(label_get_base(back_status_label));

        data->back_status_percent_label = label_alloc(flex_layout_get_base(back_status_line_flex));
        widget_set_size_content(label_get_base(data->back_status_percent_label));

        data->back_progress_bar = progress_bar_alloc(flex_layout_get_base(data->back_flex));
        widget_set_height(progress_bar_get_base(data->back_progress_bar), 8);

        data->back_extras_label = label_alloc(flex_layout_get_base(data->back_flex));
        label_set_text_font_size(data->back_extras_label, LabelFontSizeSmall);
        label_set_text_color(
            data->back_extras_label, (Color)COLOR_MAKE_HEX(BACK_EXTRAS_LABEL_COLOR));

        /* front ui */
        data->front_flex = flex_layout_alloc(instance->front_container, FlexLayoutTypeColumn);
        flex_layout_set_spacing(data->front_flex, 2);
        flex_layout_set_align(
            data->front_flex, FlexLayoutAlignCenter, FlexLayoutAlignStart, FlexLayoutAlignStart);

        Widget* front_status_line_widget = widget_alloc(flex_layout_get_base(data->front_flex));
        widget_set_height_content(front_status_line_widget);

        Label* front_status_label = label_alloc(front_status_line_widget);
        label_set_text(front_status_label, "Installing");
        widget_set_align(label_get_base(front_status_label), AlignLeftMid);

        data->front_status_percent_label = label_alloc(front_status_line_widget);
        widget_set_align(label_get_base(data->front_status_percent_label), AlignRightMid);

        data->front_progress_bar = progress_bar_alloc(flex_layout_get_base(data->front_flex));
        widget_set_height(progress_bar_get_base(data->front_progress_bar), 4);
    });
}

static void updater_scene_install_on_exit(void* context) {
    furi_assert(context);

    UpdaterApp* instance = context;
    UpdaterSceneInstall* data =
        scene_manager_get_scene_data(instance->scene_manager, UpdaterAppSceneIdInstall);

    with_gui(instance->gui, {
        flex_layout_free(data->back_flex);
        flex_layout_free(data->front_flex);
    });
}

static bool updater_scene_install_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    if(event->type == SceneManagerEventTypeCustom) {
        UpdaterApp* instance = context;
        UpdaterSceneInstall* data =
            scene_manager_get_scene_data(instance->scene_manager, UpdaterAppSceneIdInstall);

        with_gui(instance->gui, {
            /* back ui */
            label_set_text_fmt(
                data->back_status_percent_label, "(%u%%)", instance->update_percent);
            progress_bar_set_value(data->back_progress_bar, instance->update_percent);
            label_set_text(data->back_extras_label, instance->update_status);

            /* front ui */
            label_set_text_fmt(data->front_status_percent_label, "%u%%", instance->update_percent);
            progress_bar_set_value(data->front_progress_bar, instance->update_percent);
        });
    }

    return true;
}

const Scene updater_scene_install = {
    .enter_callback = updater_scene_install_on_enter,
    .exit_callback = updater_scene_install_on_exit,
    .event_callback = updater_scene_install_on_event,
    .data_size = sizeof(UpdaterSceneInstall),
};
