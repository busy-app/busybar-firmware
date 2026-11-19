#include "../../system_settings.h"

#include <settings_helpers/gui_params.h>

#include <gui/modules/label.h>
#include <gui/modules/status_view.h>
#include <gui/modules/qr_code.h>
#include <gui/modules/image.h>

typedef struct {
    StatusView* front_status_view;

    Image* info_icon;
    Label* main_label;
    Label* url_label;

    QRCode* qr_code;
} SystemSettingsSceneOffBehaviorInfo;

static void system_settings_scene_off_behavior_info_on_enter(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;

    SystemSettingsSceneOffBehaviorInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehaviorInformation);

    system_settings_show_location(instance, false);

    with_gui(instance->gui, {
        scene->front_status_view = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(
            scene->front_status_view, SHARED_IMG_PATH("info_front_8x8.image"), false);
        status_view_set_primary_text(scene->front_status_view, "Look at back\nscreen");

        Widget* back_root = instance->back_scene_window;

        scene->info_icon = image_alloc(back_root);
        image_set_source(scene->info_icon, SHARED_IMG_PATH("info_back_11x11.image"));
        widget_set_align(image_get_base(scene->info_icon), AlignTopLeft);

        scene->main_label = label_alloc(back_root);
        label_set_text(scene->main_label, "Learn more\nabout OFF\nmodes");
        label_set_font(scene->main_label, FONT_BUSY_REGULAR_9);
        widget_set_size_content(label_get_base(scene->main_label));
        widget_set_align(label_get_base(scene->main_label), AlignLeftMid);

        scene->url_label = label_alloc(back_root);
        label_set_text(scene->url_label, "docs.busy.app");
        label_set_font(scene->url_label, FONT_BUSY_REGULAR_7);
        label_set_text_color(scene->url_label, (Color)COLOR_MAKE_HEX(0x888888));
        widget_set_size_content(label_get_base(scene->url_label));
        widget_set_align(label_get_base(scene->url_label), AlignBottomLeft);

        scene->qr_code = qr_code_alloc(back_root);
        qr_code_set_size(scene->qr_code, 66);
        qr_code_set_data(
            scene->qr_code,
            "https://docs.busy.app"); // TODO: set more specific link when documentation arrives
        widget_set_size_content(qr_code_get_base(scene->qr_code));
        widget_set_padding(qr_code_get_base(scene->qr_code), 2, 2, 2, 2);
        widget_set_align(qr_code_get_base(scene->qr_code), AlignRightMid);
    });
}

static void system_settings_scene_off_behavior_info_on_exit(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SystemSettingsSceneOffBehaviorInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehaviorInformation);

    with_gui(instance->gui, {
        qr_code_free(scene->qr_code);

        label_free(scene->url_label);
        label_free(scene->main_label);
        image_free(scene->info_icon);

        status_view_free(scene->front_status_view);
    });

    system_settings_show_location(instance, true);
}

static bool system_settings_scene_off_behavior_info_on_event(
    const SceneManagerEvent* event,
    void* context) {
    furi_assert(context);
    SystemSettings* instance = context;

    bool consumed = false;
    UNUSED(instance);
    UNUSED(event);

    return consumed;
}

const Scene system_settings_scene_off_behavior_info = {
    .enter_callback = system_settings_scene_off_behavior_info_on_enter,
    .exit_callback = system_settings_scene_off_behavior_info_on_exit,
    .event_callback = system_settings_scene_off_behavior_info_on_event,
    .data_size = sizeof(SystemSettingsSceneOffBehaviorInfo),
};
