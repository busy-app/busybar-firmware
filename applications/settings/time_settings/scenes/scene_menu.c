#include "../time_settings.h"
#include <settings_helpers/gui_params.h>
#include <furi_hal_rtc.h>

#include <gui/modules/submenu.h>

#define TAG "TIME_MENU"

typedef enum {
    SceneEventTimezoneSelected,
    SceneEventFormatSelected,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} SettingsSceneMenu;

enum MenuId {
    IdxTimeZone,
    IdxTimeFormat,
};

static void scene_menu_on_submenu_item(uint32_t index, void* context) {
    TimeSettingsApp* instance = context;
    UNUSED(instance);

    switch(index) {
    case IdxTimeZone:
        time_settings_send_custom_event(instance, SceneEventTimezoneSelected);
        break;
    case IdxTimeFormat:
        time_settings_send_custom_event(instance, SceneEventFormatSelected);
        break;
    default:
        break;
    }
}

static void scene_menu_on_enter(void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;
    SettingsSceneMenu* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMenu);

    TimeSettings time_settings;
    time_get_settings(instance->time, &time_settings);

    DateTime now = furi_hal_rtc_get_datetime().dt;
    const char* letters = utz_get_current_offset(&time_settings.timezone, &now, &(utz_offset_t){});

    char zone_abbr[8];
    snprintf(zone_abbr, sizeof(zone_abbr), time_settings.timezone.abrev_formatter, letters);

    char tz_text[12];
    snprintf(tz_text, sizeof(tz_text), "%s\u200A>", zone_abbr);

    char time_format_text[10];
    snprintf(
        time_format_text,
        sizeof(time_format_text),
        "%s\u200A>",
        time_settings_format_names[time_settings.time_format]);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "Time zone",
            tz_text,
            IdxTimeZone,
            scene_menu_on_submenu_item,
            instance);
        submenu_add_item(
            data->front_menu,
            "Time format",
            time_format_text,
            IdxTimeFormat,
            scene_menu_on_submenu_item,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "Time zone", tz_text, IdxTimeZone, NULL, instance);
        submenu_add_item(
            data->back_menu, "Time format", time_format_text, IdxTimeFormat, NULL, instance);
    });
}

static void scene_menu_on_exit(void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;
    SettingsSceneMenu* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMenu);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool scene_menu_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventTimezoneSelected:
            scene_manager_next_scene(instance->scene_manager, SceneIdTimezone);
            consumed = true;
            break;
        case SceneEventFormatSelected:
            scene_manager_next_scene(instance->scene_manager, SceneIdFormat);
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene time_scene_menu = {
    .enter_callback = scene_menu_on_enter,
    .exit_callback = scene_menu_on_exit,
    .event_callback = scene_menu_on_event,
    .data_size = sizeof(SettingsSceneMenu),
};
