#include "../about.h"

#include <gui/modules/label.h>
#include <version/version.h>
#include <web_server/http_api/http_api.h>

#define GREY_TEXT(text) "#888888 " text "#"

#define SECONDS_IN_MINUTE (60U)
#define MINUTES_IN_HOUR   (60U)
#define HOURS_IN_DAY      (24U)

typedef struct {
    Label* front_label;
    Label* back_label;
} AboutSceneFirmware;

static void about_scene_firmware_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneFirmware* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFirmware);

    FuriString* firmware_info_string = furi_string_alloc();

    const Version* version = version_get();
    furi_string_printf(
        firmware_info_string, GREY_TEXT("Version:") " %s\n", version_get_version(version));
    furi_string_cat_printf(
        firmware_info_string, GREY_TEXT("Branch:") "\n%s\n", version_get_gitbranch(version));
    furi_string_cat_printf(
        firmware_info_string, GREY_TEXT("Commit hash:") "\n%s\n", version_get_githash(version));

    uint8_t api_version[] = API_VERSION;
    furi_string_cat_printf(
        firmware_info_string,
        GREY_TEXT("API version:") " %u.%u.%u\n",
        api_version[0],
        api_version[1],
        api_version[2]);
    furi_string_cat_printf(
        firmware_info_string, GREY_TEXT("Build date:") "\n%s\n", version_get_builddate(version));

    uint32_t uptime_seconds = furi_get_tick() / furi_kernel_get_tick_frequency();
    uint32_t uptime_minutes = uptime_seconds / SECONDS_IN_MINUTE;
    uint32_t uptime_hours = uptime_minutes / MINUTES_IN_HOUR;
    uint32_t uptime_days = uptime_hours / HOURS_IN_DAY;
    furi_string_cat_printf(
        firmware_info_string,
        GREY_TEXT("Uptime:") " %lud %02luh %02lum\n",
        uptime_days,
        uptime_hours % HOURS_IN_DAY,
        uptime_minutes % MINUTES_IN_HOUR);

    with_gui(instance->gui, {
        widget_set_scrollbar_mode(instance->front_scene_window, WidgetScrollBarModeAuto);

        scene->front_label = label_alloc(instance->front_scene_window);
        label_set_inline_text_color_formatting(scene->front_label, true);
        label_set_text(scene->front_label, furi_string_get_cstr(firmware_info_string));
        label_set_font(scene->front_label, FONT_BUSY_REGULAR_5);
        label_set_line_spacing(scene->front_label, -2);

        widget_set_scrollbar_mode(instance->back_scene_window, WidgetScrollBarModeAuto);

        scene->back_label = label_alloc(instance->back_scene_window);
        widget_set_padding(label_get_base(scene->back_label), 2, 0, 0, 0);
        label_set_inline_text_color_formatting(scene->back_label, true);
        label_set_text(scene->back_label, furi_string_get_cstr(firmware_info_string));
        label_set_font(scene->back_label, FONT_BUSY_REGULAR_7);
    });

    furi_string_free(firmware_info_string);
}

static void about_scene_firmware_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneFirmware* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFirmware);

    with_gui(instance->gui, {
        label_free(scene->front_label);
        widget_set_scrollbar_mode(instance->front_scene_window, WidgetScrollBarModeOff);

        label_free(scene->back_label);
        widget_set_scrollbar_mode(instance->back_scene_window, WidgetScrollBarModeOff);
    });
}

static bool about_scene_firmware_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    About* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeBack) {
        about_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene about_scene_firmware = {
    .enter_callback = about_scene_firmware_on_enter,
    .exit_callback = about_scene_firmware_on_exit,
    .event_callback = about_scene_firmware_on_event,
    .data_size = sizeof(AboutSceneFirmware),
};
