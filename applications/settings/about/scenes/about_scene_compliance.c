#include "../about.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/status_view.h>

#include <furi_hal_version.h>

#define GREY_TEXT(text) "#888888 " text "#"
#define IMAGES_NUM      (5)

static const char* image_path[IMAGES_NUM] = {
    IMG_PATH("about_fcc_back_24x24.image"),
    IMG_PATH("about_ce_back_24x24.image"),
    IMG_PATH("about_uk_back_24x24.image"),
    IMG_PATH("about_dispose_back_24x24.image"),
    IMG_PATH("about_rohs_back_24x24.image"),
};

typedef struct {
    StatusView* front_status_view;
    FlexLayout* info_flex;
    Label* compliance_info_label;
    Label* compliance_message_label;
    FlexLayout* images_flex;
    Image* images[IMAGES_NUM];
} AboutSceneCompliance;

static void about_scene_compliance_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneCompliance* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdCompliance);
    FuriString* compliance_info_string = furi_string_alloc();
    furi_string_printf(compliance_info_string, GREY_TEXT("Product:") " BUSY Bar\n");
    furi_string_cat_printf(compliance_info_string, GREY_TEXT("Model/HVIN:") " BB.1\n");
    furi_string_cat_printf(
        compliance_info_string, GREY_TEXT("FCC ID:") " %s\n", furi_hal_version_get_fcc_id());
    furi_string_cat_printf(
        compliance_info_string, GREY_TEXT("IC:") " %s", furi_hal_version_get_ic_id());

    FuriString* compliance_message_string = furi_string_alloc();
    furi_string_printf(
        compliance_message_string,
        GREY_TEXT("For all compliance") "\n" GREY_TEXT(
            "certificates visit:") "\nwww.busy.app/bar/compliance\n");

    with_gui(instance->gui, {
        scene->front_status_view = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(
            scene->front_status_view, SHARED_IMG_PATH("info_front_8x8.image"), false);
        status_view_set_primary_text(scene->front_status_view, "Look at back\nscreen");

        scene->info_flex = flex_layout_alloc(instance->back_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_spacing(scene->info_flex, 3);
        widget_set_padding(flex_layout_get_base(scene->info_flex), 2, 0, 0, 0);

        Widget* info_flex_base = flex_layout_get_base(scene->info_flex);
        widget_set_scrollbar_mode(info_flex_base, WidgetScrollBarModeAuto);

        scene->compliance_info_label = label_alloc(info_flex_base);
        label_set_inline_text_color_formatting(scene->compliance_info_label, true);
        label_set_text(scene->compliance_info_label, furi_string_get_cstr(compliance_info_string));
        label_set_font(scene->compliance_info_label, FONT_BUSY_REGULAR_7);

        scene->images_flex = flex_layout_alloc(info_flex_base, FlexLayoutTypeRow);
        Widget* images_flex_base = flex_layout_get_base(scene->images_flex);
        widget_set_height_content(images_flex_base);
        for(size_t i = 0; i < IMAGES_NUM; i++) {
            scene->images[i] = image_alloc(images_flex_base);
            image_set_source(scene->images[i], image_path[i]);
            widget_set_size_content(image_get_base(scene->images[i]));
            flex_layout_set_child_widget_grow(
                scene->images_flex, image_get_base(scene->images[i]), 1);
        }

        scene->compliance_message_label = label_alloc(info_flex_base);
        label_set_inline_text_color_formatting(scene->compliance_message_label, true);
        label_set_text(
            scene->compliance_message_label, furi_string_get_cstr(compliance_message_string));
        label_set_font(scene->compliance_message_label, FONT_BUSY_REGULAR_7);
    });

    furi_string_free(compliance_message_string);
    furi_string_free(compliance_info_string);
}

static void about_scene_compliance_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneCompliance* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdCompliance);

    with_gui(instance->gui, {
        status_view_free(scene->front_status_view);
        label_free(scene->compliance_info_label);
        label_free(scene->compliance_message_label);
        for(size_t i = 0; i < IMAGES_NUM; i++) {
            image_free(scene->images[i]);
        }
        flex_layout_free(scene->images_flex);
        flex_layout_free(scene->info_flex);
    });
}

static bool about_scene_compliance_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    About* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeBack) {
        about_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene about_scene_compliance = {
    .enter_callback = about_scene_compliance_on_enter,
    .exit_callback = about_scene_compliance_on_exit,
    .event_callback = about_scene_compliance_on_event,
    .data_size = sizeof(AboutSceneCompliance),
};
