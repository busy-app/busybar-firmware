#include "../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#include <furi_hal_nvm.h>

typedef enum {
    SceneEventDebugAppsChanged = AppEventSceneEventsStart,
} SceneEvent;

typedef enum {
    DebugAppsOff,
    DebugAppsOn,

    DebugAppsCount,
} DebugAppsState;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;

    _Atomic DebugAppsState debug_apps;
} SettingsSceneDebug;

static const char* debug_apps_names[DebugAppsCount] = {
    [DebugAppsOff] = "Off",
    [DebugAppsOn] = "On",
};

static void system_settings_scene_debug_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneDebug* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdDebug);

    data->debug_apps = var_item_get_value(item);
    system_settings_send_custom_event(instance, SceneEventDebugAppsChanged);
}

static void system_settings_scene_debug_fill_var_item_list(
    SystemSettings* instance,
    VarItemList* list,
    bool do_set_callbacks) {
    SettingsSceneDebug* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdDebug);

    VarItem* debug_item = var_item_list_add_selector(
        list,
        "Debug apps",
        NULL,
        debug_apps_names,
        COUNT_OF(debug_apps_names),
        (do_set_callbacks) ? system_settings_scene_debug_changed_callback : NULL,
        instance);

    var_item_set_value(debug_item, data->debug_apps);
}

static void system_settings_scene_debug_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneDebug* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdDebug);

    data->debug_apps = furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug) ? DebugAppsOn : DebugAppsOff;

    with_gui(instance->gui, {
        nav_bar_push_location(instance->back_nav_bar, "DEBUG");

        data->front_list = var_item_list_alloc(instance->front_scene_window);
        system_settings_scene_debug_fill_var_item_list(instance, data->front_list, true);

        data->back_list = var_item_list_alloc(instance->back_scene_window);
        system_settings_scene_debug_fill_var_item_list(instance, data->back_list, false);
    });
}

static void system_settings_scene_debug_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneDebug* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdDebug);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->back_nav_bar);

        var_item_list_free(data->front_list);
        var_item_list_free(data->back_list);
    });
}

static bool system_settings_scene_debug_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventDebugAppsChanged) {
            SettingsSceneDebug* data =
                scene_manager_get_scene_data(instance->scene_manager, SceneIdDebug);
            if(data->debug_apps == DebugAppsOff) {
                furi_hal_nvm_reset_flag(FuriHalNvmFlagDebug);
            } else if(data->debug_apps == DebugAppsOn) {
                furi_hal_nvm_set_flag(FuriHalNvmFlagDebug);
            }
        }
        consumed = true;
    }

    return consumed;
}

const Scene system_settings_scene_debug = {
    .enter_callback = system_settings_scene_debug_on_enter,
    .exit_callback = system_settings_scene_debug_on_exit,
    .event_callback = system_settings_scene_debug_on_event,
    .data_size = sizeof(SettingsSceneDebug),
};
