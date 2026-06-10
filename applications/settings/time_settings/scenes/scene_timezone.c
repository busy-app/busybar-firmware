#include "../time_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/submenu.h>
#include <tzutil.h>
#include <furi.h>
#include <furi_hal_rtc.h>

#define TAG "TZ_SCENE"

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;

    TzutilTzInfoList list;
} SettingsSceneTimezone;

static void scene_timezone_on_submenu_item(uint32_t index, void* context) {
    TimeSettingsApp* instance = context;
    time_settings_send_custom_event(instance, index);
}

static void scene_timezone_fill_submenu(
    TimeSettingsApp* instance,
    Submenu* menu,
    const TzutilTzInfoList* list,
    bool do_set_callbacks,
    size_t selected_index) {
    for(size_t i = 0; i != list->count; ++i) {
        char offset_buf[DATETIME_OFFSET_STR_LEN + 1];
        datetime_format_offset(&list->entries[i].offset, offset_buf);
        FuriString* s = furi_string_alloc_printf("UTC%s, %s", offset_buf, list->entries[i].name);
        submenu_add_item(
            menu,
            furi_string_get_cstr(s),
            NULL,
            i,
            do_set_callbacks ? scene_timezone_on_submenu_item : NULL,
            instance);
        furi_string_free(s);
    }
    submenu_set_selected_item_index(menu, selected_index);
}

static size_t get_current_timezone_index(TimeSettingsApp* instance) {
    SettingsSceneTimezone* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTimezone);
    TimeSettings time_settings;
    time_get_settings(instance->time, &time_settings);
    for(size_t i = 0; i != data->list.count; ++i) {
        // Ok to compare pointers: they point to the same table
        if(data->list.entries[i].name == time_settings.timezone.name) {
            return i;
        }
    }
    return 0;
}

static void scene_timezone_on_enter(void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;
    SettingsSceneTimezone* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTimezone);

    DateTime now = furi_hal_rtc_get_datetime().dt;
    data->list = tzutil_compile_zone_list(&now);

    size_t selected_index = get_current_timezone_index(instance);

    with_gui(instance->gui, {
        nav_bar_push_location(instance->back_nav_bar, "TIME ZONE");
        data->front_menu = submenu_alloc(instance->front_scene_window);
        scene_timezone_fill_submenu(instance, data->front_menu, &data->list, true, selected_index);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        scene_timezone_fill_submenu(instance, data->back_menu, &data->list, false, selected_index);

        widget_set_scrollbar_enabled(submenu_get_base(data->front_menu), true);
        widget_set_scrollbar_enabled(submenu_get_base(data->back_menu), true);
    });
}

static void scene_timezone_on_exit(void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;
    SettingsSceneTimezone* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTimezone);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->back_nav_bar);
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
    tzutil_info_list_free(&data->list);
}

static bool scene_timezone_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    TimeSettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        SettingsSceneTimezone* data =
            scene_manager_get_scene_data(instance->scene_manager, SceneIdTimezone);
        consumed = true;

        const char* zone_name = data->list.entries[event->event].name;
        FURI_LOG_D(TAG, "Selected: %s", zone_name);
        TimeSettings time_settings;
        time_get_settings(instance->time, &time_settings);
        bool ok = utz_get_zone_by_name(zone_name, &time_settings.timezone);
        ok = ok && time_set_settings(instance->time, &time_settings);
        if(!ok) {
            FURI_LOG_E(TAG, "Error setting timezone");
        }
        scene_manager_previous_scene(instance->scene_manager);
    } else if(event->type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(instance->scene_manager);
        consumed = true;
    }

    return consumed;
}

const Scene time_scene_timezone = {
    .enter_callback = scene_timezone_on_enter,
    .exit_callback = scene_timezone_on_exit,
    .event_callback = scene_timezone_on_event,
    .data_size = sizeof(SettingsSceneTimezone),
};
