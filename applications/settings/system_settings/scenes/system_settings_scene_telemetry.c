#include "../system_settings.h"

#include <gui/modules/var_item_list.h>
#include <telemetry/telemetry.h>

typedef enum {
    SceneEventTelemetryChanged = AppEventSceneEventsStart,
} SceneEvent;

typedef enum {
    TelemetryStateOff,
    TelemetryStateOn,

    TelemetryStateCount,
} TelemetryState;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;

    _Atomic TelemetryState telemetry_enabled;
} SettingsSceneTelemetry;

static const char* telemetry_state_names[TelemetryStateCount] = {
    [TelemetryStateOff] = "Off",
    [TelemetryStateOn] = "On",
};

static void system_settings_scene_telemetry_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneTelemetry* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTelemetry);

    data->telemetry_enabled = var_item_get_value(item);
    system_settings_send_custom_event(instance, SceneEventTelemetryChanged);
}

static void system_settings_scene_telemetry_fill_var_item_list(
    SystemSettings* instance,
    VarItemList* list,
    bool do_set_callbacks) {
    SettingsSceneTelemetry* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTelemetry);

    VarItem* telemetry_item = var_item_list_add_selector(
        list,
        "Telemetry",
        NULL,
        telemetry_state_names,
        COUNT_OF(telemetry_state_names),
        (do_set_callbacks) ? system_settings_scene_telemetry_changed_callback : NULL,
        instance);

    var_item_set_value(telemetry_item, data->telemetry_enabled);
}

static void system_settings_scene_telemetry_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneTelemetry* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTelemetry);

    Telemetry* telemetry = furi_record_open(RECORD_TELEMETRY);
    data->telemetry_enabled = telemetry_is_enabled(telemetry) ? TelemetryStateOn :
                                                                TelemetryStateOff;
    furi_record_close(RECORD_TELEMETRY);

    with_gui(instance->gui, {
        data->front_list = var_item_list_alloc(instance->front_scene_window);
        system_settings_scene_telemetry_fill_var_item_list(instance, data->front_list, true);

        data->back_list = var_item_list_alloc(instance->back_scene_window);
        system_settings_scene_telemetry_fill_var_item_list(instance, data->back_list, false);

        widget_set_scrollbar_enabled(var_item_list_get_base(data->front_list), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(data->back_list), true);
    });
}

static void system_settings_scene_telemetry_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SettingsSceneTelemetry* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdTelemetry);

    with_gui(instance->gui, {
        var_item_list_free(data->front_list);
        var_item_list_free(data->back_list);
    });
}

static bool
    system_settings_scene_telemetry_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventTelemetryChanged) {
            SettingsSceneTelemetry* data =
                scene_manager_get_scene_data(instance->scene_manager, SceneIdTelemetry);

            Telemetry* telemetry = furi_record_open(RECORD_TELEMETRY);
            telemetry_set_enabled(telemetry, data->telemetry_enabled == TelemetryStateOn);
            furi_record_close(RECORD_TELEMETRY);
        }
        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        system_settings_pop_location(instance);
        scene_manager_previous_scene(instance->scene_manager);
        consumed = true;
    }

    return consumed;
}

const Scene system_settings_scene_telemetry = {
    .enter_callback = system_settings_scene_telemetry_on_enter,
    .exit_callback = system_settings_scene_telemetry_on_exit,
    .event_callback = system_settings_scene_telemetry_on_event,
    .data_size = sizeof(SettingsSceneTelemetry),
};
