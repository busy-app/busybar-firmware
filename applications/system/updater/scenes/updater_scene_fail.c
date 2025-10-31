#include "../update_app_i.h"
#include "../storage_macros.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>

#define BACK_EXTRAS_LABEL_COLOR 0x777777

typedef struct {
    FlexLayout* back_flex;
    Label* back_extras_label;

    FlexLayout* front_flex;
} UpdaterSceneFail;

static void updater_scene_fail_on_enter(void* context) {
    furi_assert(context);

    UpdaterApp* instance = context;
    UpdaterSceneFail* data = scene_manager_get_current_scene_data(instance->scene_manager);

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
        image_set_source(back_status_image, UPDATER_IMG_PATH("fail_back_12x12.bin"));
        widget_set_size_content(image_get_base(back_status_image));

        Label* back_status_label = label_alloc(flex_layout_get_base(back_status_line_flex));
        label_set_text(back_status_label, "Update failed");
        widget_set_size_content(label_get_base(back_status_label));

        Label* back_description_label = label_alloc(flex_layout_get_base(data->back_flex));
        label_set_long_content_mode(back_description_label, LabelLongContentModeWrap, 0);
        label_set_text_font_size(back_description_label, LabelFontSizeSmall);
        label_set_text(
            back_description_label, "An error occurred during the update. Try again later.");

        data->back_extras_label = label_alloc(flex_layout_get_base(data->back_flex));
        label_set_text_font_size(data->back_extras_label, LabelFontSizeSmall);
        label_set_text_color(
            data->back_extras_label, (Color)COLOR_MAKE_HEX(BACK_EXTRAS_LABEL_COLOR));

        /* front ui */
        data->front_flex = flex_layout_alloc(instance->front_container, FlexLayoutTypeRow);
        flex_layout_set_spacing(data->front_flex, 2);
        widget_set_size_content(flex_layout_get_base(data->front_flex));
        widget_set_align(flex_layout_get_base(data->front_flex), AlignLeftMid);

        Image* front_status_image = image_alloc(flex_layout_get_base(data->front_flex));
        image_set_source(front_status_image, UPDATER_IMG_PATH("fail_front_8x8.bin"));
        widget_set_size_content(image_get_base(front_status_image));

        Label* front_status_label = label_alloc(flex_layout_get_base(data->front_flex));
        label_set_text(front_status_label, "Update failed");
        widget_set_size_content(label_get_base(front_status_label));
    });
}

static void updater_scene_fail_on_exit(void* context) {
    furi_assert(context);

    UpdaterApp* instance = context;
    UpdaterSceneFail* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        flex_layout_free(data->back_flex);
        flex_layout_free(data->front_flex);
    });
}

static bool updater_scene_fail_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event->type == SceneManagerEventTypeCustom);

    UpdaterApp* instance = context;
    UpdaterSceneFail* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        /* back ui */
        label_set_text(data->back_extras_label, instance->update_status);
    });

    return true;
}

const Scene updater_scene_fail = {
    .enter_callback = updater_scene_fail_on_enter,
    .exit_callback = updater_scene_fail_on_exit,
    .event_callback = updater_scene_fail_on_event,
    .data_size = sizeof(UpdaterSceneFail),
};
