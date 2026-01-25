#include "../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventFactoryReset = AppEventSceneEventsStart,
    SceneEventPower,
    SceneEventDebug,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} SettingsSceneSystem;

static void scene_main_menu_item_callback(uint32_t index, void* context) {
    system_settings_send_custom_event(context, index);
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "Factory reset",
            SceneEventFactoryReset,
            scene_main_menu_item_callback,
            instance);

        submenu_add_item(
            data->front_menu, "Power", SceneEventPower, scene_main_menu_item_callback, instance);

        submenu_add_item(
            data->front_menu,
            "Debug apps",
            SceneEventDebug,
            scene_main_menu_item_callback,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "Factory reset", SceneEventFactoryReset, NULL, instance);

        submenu_add_item(data->back_menu, "Power", SceneEventPower, NULL, instance);

        submenu_add_item(data->back_menu, "Debug apps", SceneEventDebug, NULL, instance);
    });
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneSystem* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene system_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(SettingsSceneSystem),
};
