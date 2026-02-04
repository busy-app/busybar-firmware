#include "../about.h"

#include <settings_helpers/gui_params.h>
#include <settings_helpers/status_view.h>

#include <gui/modules/label.h>
#include <gui/modules/image.h>

#define GREY_TEXT(text) "#888888 " text "#"
#define IMAGES_NUM      (5)

static const char* image_path[IMAGES_NUM] = {
    IMG_PATH("about_fcc_back_24x24.bin"),
    IMG_PATH("about_ce_back_24x24.bin"),
    IMG_PATH("about_uk_back_24x24.bin"),
    IMG_PATH("about_dispose_back_24x24.bin"),
    IMG_PATH("about_rohs_back_24x24.bin"),
};

typedef struct {
    StatusView* front_status_view;

    FlexLayout* info_flex;

    Label* complience_info_label;
    FuriString* complience_info_str;

    Label* complience_message_label;
    FuriString* complience_message_str;

    FlexLayout* images_flex;
    Image* images[IMAGES_NUM];
} AboutSceneComplience;

static void about_scene_complience_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneComplience* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdComplience);
    scene->complience_info_str = furi_string_alloc();
    furi_string_printf(scene->complience_info_str, GREY_TEXT("Product:") " BUSY Bar\n");
    furi_string_cat_printf(scene->complience_info_str, GREY_TEXT("Model:") " BB.1\n");
    furi_string_cat_printf(scene->complience_info_str, GREY_TEXT("FCC ID:") " TODO\n");
    furi_string_cat_printf(scene->complience_info_str, GREY_TEXT("IC:") " TODO");

    scene->complience_message_str = furi_string_alloc();
    furi_string_printf(
        scene->complience_message_str,
        GREY_TEXT("For all complience") "\n" GREY_TEXT(
            "certificates visit:") "\nwww.busy.app/bar/complience\n");

    with_gui(instance->gui, {
        scene->front_status_view = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(scene->front_status_view, SETTINGS_IMG_PATH("info_front_7x7.bin"));
        status_view_set_header(scene->front_status_view, "Look at back\nscreen");

        scene->info_flex = flex_layout_alloc(instance->back_scene_window, FlexLayoutTypeColumn);
        flex_layout_set_spacing(scene->info_flex, 3);
        Widget* info_flex_base = flex_layout_get_base(scene->info_flex);
        widget_set_scrollbar_mode(info_flex_base, WidgetScrollBarModeAuto);

        scene->complience_info_label = label_alloc(info_flex_base);
        label_set_inline_text_color_formatting(scene->complience_info_label, true);
        label_set_text(
            scene->complience_info_label, furi_string_get_cstr(scene->complience_info_str));

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

        scene->complience_message_label = label_alloc(info_flex_base);
        label_set_inline_text_color_formatting(scene->complience_message_label, true);
        label_set_text(
            scene->complience_message_label, furi_string_get_cstr(scene->complience_message_str));
    });
}

static void about_scene_complience_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneComplience* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdComplience);

    furi_string_free(scene->complience_info_str);
    furi_string_free(scene->complience_message_str);

    with_gui(instance->gui, {
        status_view_free(scene->front_status_view);
        label_free(scene->complience_info_label);
        label_free(scene->complience_message_label);
        for(size_t i = 0; i < IMAGES_NUM; i++) {
            image_free(scene->images[i]);
        }
        flex_layout_free(scene->images_flex);
        flex_layout_free(scene->info_flex);
    });
}

static bool about_scene_complience_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    About* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeBack) {
        about_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene about_scene_complience = {
    .enter_callback = about_scene_complience_on_enter,
    .exit_callback = about_scene_complience_on_exit,
    .event_callback = about_scene_complience_on_event,
    .data_size = sizeof(AboutSceneComplience),
};
