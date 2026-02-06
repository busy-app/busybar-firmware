#include "../about.h"

#include <gui/modules/label.h>
#include <version/version.h>
#include <web_server/http_api/http_api.h>

#define GREY_TEXT(text) "#888888 " text "#"

typedef struct {
    Label* firmware_info[GuiDisplayIdMax];
    FuriString* firmware_info_str;
} AboutSceneFirmware;

static void about_scene_firmware_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneFirmware* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFirmware);
    scene->firmware_info_str = furi_string_alloc();

    const Version* version = version_get();
    furi_string_printf(
        scene->firmware_info_str, GREY_TEXT("Version:") " %s\n", version_get_version(version));
    furi_string_cat_printf(
        scene->firmware_info_str, GREY_TEXT("Branch:") "\n%s\n", version_get_gitbranch(version));
    furi_string_cat_printf(
        scene->firmware_info_str,
        GREY_TEXT("Commit hash:") "\n%s\n",
        version_get_githash(version));

    uint8_t api_version[] = API_VERSION;
    furi_string_cat_printf(
        scene->firmware_info_str,
        GREY_TEXT("API version:") " %u.%u.%u\n",
        api_version[0],
        api_version[1],
        api_version[2]);
    furi_string_cat_printf(
        scene->firmware_info_str,
        GREY_TEXT("Build date:") "\n%s\n",
        version_get_builddate(version));

    uint32_t uptime_s = furi_get_tick() / furi_kernel_get_tick_frequency();
    uint32_t minutes = uptime_s / 60;
    uint32_t hours = minutes / 60;
    uint32_t days = hours / 24;
    furi_string_cat_printf(
        scene->firmware_info_str,
        GREY_TEXT("Uptime:") " %lud %02luh %02lum\n",
        days,
        hours,
        minutes);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            widget_set_scrollbar_mode(windows[disp], WidgetScrollBarModeAuto);
            scene->firmware_info[disp] = label_alloc(windows[disp]);
            label_set_inline_text_color_formatting(scene->firmware_info[disp], true);
            label_set_text(
                scene->firmware_info[disp], furi_string_get_cstr(scene->firmware_info_str));
        }
    });
}

static void about_scene_firmware_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneFirmware* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFirmware);

    furi_string_free(scene->firmware_info_str);

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            label_free(scene->firmware_info[disp]);
        }
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
