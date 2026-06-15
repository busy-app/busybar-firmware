#include "../firmware_i.h"

#include <gui/modules/label.h>
#include <version/version.h>
#include <web_server/http_api/http_api.h>

#define GREY_TEXT(text) "#888888 " text "#"

typedef struct {
    Label* front_label;
    Label* back_label;
} FirmwareSettingsVersionInfoScene;

static void firmware_settings_scene_version_info_on_enter(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsVersionInfoScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxVersionInfo);

    FuriString* version_info_string = furi_string_alloc();

    const Version* version = version_get();
    furi_string_printf(
        version_info_string, GREY_TEXT("Version:") " %s\n", version_get_version(version));
    furi_string_cat_printf(
        version_info_string, GREY_TEXT("Branch:") "\n%s\n", version_get_gitbranch(version));
    furi_string_cat_printf(
        version_info_string, GREY_TEXT("Commit hash:") "\n%s\n", version_get_githash(version));

    uint8_t api_version[] = API_VERSION;
    furi_string_cat_printf(
        version_info_string,
        GREY_TEXT("API version:") " %u.%u.%u\n",
        api_version[0],
        api_version[1],
        api_version[2]);
    furi_string_cat_printf(
        version_info_string, GREY_TEXT("Build date:") "\n%s", version_get_builddate(version));

    with_gui(instance->gui, {
        scene->front_label = label_alloc(instance->front_scene_window);
        widget_set_padding(label_get_base(scene->front_label), 0, 2, 0, 0);
        label_set_inline_text_color_formatting(scene->front_label, true);
        label_set_text(scene->front_label, furi_string_get_cstr(version_info_string));
        label_set_font(scene->front_label, FONT_BUSY_REGULAR_5);
        label_set_line_spacing(scene->front_label, -2);

        scene->back_label = label_alloc(instance->back_scene_window);
        widget_set_padding(label_get_base(scene->back_label), 2, 4, 0, 0);
        label_set_inline_text_color_formatting(scene->back_label, true);
        label_set_text(scene->back_label, furi_string_get_cstr(version_info_string));
        label_set_font(scene->back_label, FONT_BUSY_REGULAR_7);

        widget_set_scrollbar_enabled(instance->front_scene_window, true);
        widget_set_scrollbar_enabled(instance->back_scene_window, true);
    });

    furi_string_free(version_info_string);
}

static void firmware_settings_scene_version_info_on_exit(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsVersionInfoScene* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxVersionInfo);

    with_gui(instance->gui, {
        label_free(scene->front_label);
        widget_set_scrollbar_enabled(instance->front_scene_window, false);

        label_free(scene->back_label);
        widget_set_scrollbar_enabled(instance->back_scene_window, false);
    });
}

static bool
    firmware_settings_scene_version_info_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;

    if(event->type == SceneManagerEventTypeBack) {
        with_gui(instance->gui, { nav_bar_pop_location(instance->back_nav_bar); });
    }

    return false;
}

const Scene firmware_settings_internal_version_info = {
    .enter_callback = firmware_settings_scene_version_info_on_enter,
    .exit_callback = firmware_settings_scene_version_info_on_exit,
    .event_callback = firmware_settings_scene_version_info_on_event,
    .data_size = sizeof(FirmwareSettingsVersionInfoScene),
};
