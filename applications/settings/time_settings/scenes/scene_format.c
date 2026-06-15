#include "../time_settings.h"
#include <settings_helpers/gui_params.h>
#include <furi_hal_rtc.h>

#include <gui/modules/var_item_list.h>

#define TAG "TIME_MENU"

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;
} SettingsSceneFormat;

const char* time_settings_format_names[TimeSettingTimeFormatCount] =
    {[TimeSettingTimeFormat24h] = "24h", [TimeSettingTimeFormat12h] = "12h"};

static void scene_main_on_changed(VarItem* item, void* context) {
    TimeSettingsApp* instance = context;

    TimeSettings time_settings;
    time_get_settings(instance->time, &time_settings);

    time_settings.time_format = var_item_get_value(item);

    time_set_settings(instance->time, &time_settings);
}

static void scene_format_on_enter(void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;
    SettingsSceneFormat* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdFormat);

    TimeSettings time_settings;
    time_get_settings(instance->time, &time_settings);

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
        var_item_set_value(item, time_settings.time_format);
        data->back_list = var_item_list_alloc(instance->back_scene_window);
        item = var_item_list_add_selector(
            data->back_list,
            "Time format",
            NULL,
            time_settings_format_names,
            COUNT_OF(time_settings_format_names),
            NULL,
            instance);
        var_item_set_value(item, time_settings.time_format);

        widget_set_scrollbar_enabled(var_item_list_get_base(data->front_list), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(data->back_list), true);
    });
}

static void scene_format_on_exit(void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;
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

    TimeSettingsApp* instance = context;

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
