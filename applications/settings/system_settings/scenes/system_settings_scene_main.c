#include "../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventPower = AppEventSceneEventsStart,
    SceneEventDebug,
    SceneEventTelemetry,
    SceneEventFactoryReset,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
    uint32_t menu_index;
} SettingsSceneSystem;

static void system_settings_scene_main_menu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    system_settings_send_custom_event(context, index);
}

static void system_settings_scene_main_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);

        submenu_add_item(
            data->front_menu,
            "Power",
            NULL,
            SceneEventPower,
            system_settings_scene_main_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Debug",
            NULL,
            SceneEventDebug,
            system_settings_scene_main_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Telemetry",
            NULL,
            SceneEventTelemetry,
            system_settings_scene_main_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Factory reset",
            NULL,
            SceneEventFactoryReset,
            system_settings_scene_main_menu_item_callback,
            instance);
        submenu_set_selected_item_index(data->front_menu, data->menu_index);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "Power", NULL, SceneEventPower, NULL, instance);
        submenu_add_item(data->back_menu, "Debug", NULL, SceneEventDebug, NULL, instance);
        submenu_add_item(data->back_menu, "Telemetry", NULL, SceneEventTelemetry, NULL, instance);
        submenu_add_item(
            data->back_menu, "Factory reset", NULL, SceneEventFactoryReset, NULL, instance);
        submenu_set_selected_item_index(data->back_menu, data->menu_index);

        widget_set_scrollbar_enabled(submenu_get_base(data->front_menu), true);
        widget_set_scrollbar_enabled(submenu_get_base(data->back_menu), true);
    });
}

static void system_settings_scene_main_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool system_settings_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventPower) {
            scene_manager_next_scene(instance->scene_manager, SceneIdPowerMenu);
            system_settings_push_location(instance, "POWER");
            consumed = true;
        } else if(event->event == SceneEventDebug) {
            scene_manager_next_scene(instance->scene_manager, SceneIdDebug);
            system_settings_push_location(instance, "DEBUG");
            consumed = true;
        } else if(event->event == SceneEventTelemetry) {
            scene_manager_next_scene(instance->scene_manager, SceneIdTelemetry);
            system_settings_push_location(instance, "TELEMETRY");
            consumed = true;
        } else if(event->event == SceneEventFactoryReset) {
            scene_manager_next_scene(instance->scene_manager, SceneIdFactoryResetConfirm);
            system_settings_push_location(instance, "FACTORY RESET");
            consumed = true;
        }
        data->menu_index = event->event;
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene system_settings_scene_main = {
    .enter_callback = system_settings_scene_main_on_enter,
    .exit_callback = system_settings_scene_main_on_exit,
    .event_callback = system_settings_scene_main_on_event,
    .data_size = sizeof(SettingsSceneSystem),
};
