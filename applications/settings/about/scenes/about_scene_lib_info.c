#include "../about.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/label.h>
#include <gui/modules/status_view.h>
#include <gui/modules/qr_code.h>
#include <gui/widget_i.h>

#define GREY_TEXT_COLOR ((Color)COLOR_MAKE_RGB(0x88, 0x88, 0x88))

typedef struct {
    StatusView* front_status_view;
    FlexLayout* info_flex;
    FlexLayout* text_flex;
    Label* name_label;
    Label* license_label;
    QRCode* qr_code;
} AboutSceneLibInfo;

static void about_scene_lib_info_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;
    furi_assert(instance->license_lib_index < about_get_libs_count());

    AboutSceneLibInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLibInfo);

    about_show_location(instance, false);

    const AboutLibInfo* lib_info = about_get_lib_info(instance->license_lib_index);
    FuriString* license_string = furi_string_alloc_printf("License: %s", lib_info->license);

    with_gui(instance->gui, {
        scene->front_status_view = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(
            scene->front_status_view, SHARED_IMG_PATH("info_front_8x8.image"), false);
        status_view_set_primary_text(scene->front_status_view, "Look at back\nscreen");

        Widget* back_root = instance->back_scene_window;
        scene->info_flex = flex_layout_alloc(back_root, FlexLayoutTypeRow);
        flex_layout_set_spacing(scene->info_flex, 12);
        Widget* info_flex_base = flex_layout_get_base(scene->info_flex);
        widget_set_padding(info_flex_base, 8, 8, 0, 0);

        scene->text_flex = flex_layout_alloc(info_flex_base, FlexLayoutTypeColumn);
        Widget* text_flex_base = flex_layout_get_base(scene->text_flex);
        flex_layout_set_spacing(scene->text_flex, 2);
        flex_layout_set_align(
            scene->text_flex, FlexLayoutAlignCenter, FlexLayoutAlignStart, FlexLayoutAlignStart);
        widget_set_padding(flex_layout_get_base(scene->text_flex), 0, 0, 0, 0);

        scene->name_label = label_alloc(text_flex_base);
        label_set_text(scene->name_label, lib_info->name);
        label_set_font(scene->name_label, FONT_BUSY_REGULAR_9);
        widget_set_size_content(label_get_base(scene->name_label));

        scene->license_label = label_alloc(text_flex_base);
        label_set_text(scene->license_label, furi_string_get_cstr(license_string));
        label_set_font(scene->license_label, FONT_BUSY_REGULAR_7);
        label_set_text_color(scene->license_label, GREY_TEXT_COLOR);
        widget_set_height_content(label_get_base(scene->license_label));
        widget_set_width(label_get_base(scene->license_label), LV_PCT(100));
        flex_layout_set_child_widget_grow(scene->info_flex, text_flex_base, 1);

        scene->qr_code = qr_code_alloc(info_flex_base);
        qr_code_set_size(scene->qr_code, 39);
        qr_code_set_data(scene->qr_code, lib_info->url);
        widget_set_size_content(qr_code_get_base(scene->qr_code));
        widget_set_padding(qr_code_get_base(scene->qr_code), 2, 2, 2, 2);

        flex_layout_set_align(
            scene->info_flex,
            FlexLayoutAlignSpaceBetween,
            FlexLayoutAlignCenter,
            FlexLayoutAlignCenter);
    });

    furi_string_free(license_string);
}

static void about_scene_lib_info_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneLibInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLibInfo);

    with_gui(instance->gui, {
        status_view_free(scene->front_status_view);
        label_free(scene->name_label);
        label_free(scene->license_label);
        qr_code_free(scene->qr_code);
        flex_layout_free(scene->text_flex);
        flex_layout_free(scene->info_flex);
    });

    about_show_location(instance, true);
}

static bool about_scene_lib_info_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    About* instance = context;

    bool consumed = false;
    UNUSED(instance);
    UNUSED(event);

    return consumed;
}

const Scene about_scene_lib_info = {
    .enter_callback = about_scene_lib_info_on_enter,
    .exit_callback = about_scene_lib_info_on_exit,
    .event_callback = about_scene_lib_info_on_event,
    .data_size = sizeof(AboutSceneLibInfo),
};
