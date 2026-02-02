#include "../time_settings.h"
#include <settings_helpers/gui_params.h>
#include <furi_hal_rtc.h>

#include <gui/modules/var_item_list.h>

#define TAG "TIME_MENU"

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;
} SettingsSceneFormat;

const char* time_settings_format_names[SntpSettingTimeFormatCount] =
    {[SntpSettingTimeFormat24h] = "24h", [SntpSettingTimeFormat12h] = "12h"};

static void scene_main_on_changed(VarItem* item, void* context) {
    TimeSettings* instance = context;

    SntpSettings sntp_settings;
    sntp_get_settings(instance->sntp, &sntp_settings);

    sntp_settings.time_format = var_item_get_value(item);

    sntp_set_settings(instance->sntp, &sntp_settings);
}

static void scene_format_on_enter(void* context) {
    furi_assert(context);

    TimeSettings* instance = context;
    SettingsSceneFormat* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFormat);

    SntpSettings sntp_settings;
    sntp_get_settings(instance->sntp, &sntp_settings);

    with_gui(instance->gui, {
        nav_bar_push_location(instance->back_nav_bar, "TIME FORMAT");
        data->front_list = var_item_list_alloc(instance->front_scene_window);
        VarItem* item = var_item_list_add_selector(
            data->front_list,
            "Time format",
            NULL,
            time_settings_format_names,
            COUNT_OF(time_settings_format_names),
            scene_main_on_changed,
            instance);
        var_item_set_value(item, sntp_settings.time_format);
        data->back_list = var_item_list_alloc(instance->back_scene_window);
        item = var_item_list_add_selector(
            data->back_list,
            "Time format",
            NULL,
            time_settings_format_names,
            COUNT_OF(time_settings_format_names),
            NULL,
            instance);
        var_item_set_value(item, sntp_settings.time_format);
    });
}

static void scene_format_on_exit(void* context) {
    furi_assert(context);

    TimeSettings* instance = context;
    SettingsSceneFormat* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFormat);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->back_nav_bar);
        var_item_list_free(data->front_list);
        var_item_list_free(data->back_list);
    });
}

static bool scene_format_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    TimeSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
    } else if(event->type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(instance->scene_manager);
        consumed = true;
    }

    return consumed;
}

const Scene time_scene_format = {
    .enter_callback = scene_format_on_enter,
    .exit_callback = scene_format_on_exit,
    .event_callback = scene_format_on_event,
    .data_size = sizeof(SettingsSceneFormat),
};
