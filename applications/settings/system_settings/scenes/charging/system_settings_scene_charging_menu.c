#include "../../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#include <power/power_service/power.h>

typedef enum {
    SceneEventLimitChanged = AppEventSceneEventsStart,
} SceneEvent;

typedef enum {
    VarItemListIdLimit,

    VarItemListIdsCount,
} VarItemListId;

typedef struct {
    VarItemList* front;
    VarItemList* back;

    Power* power;
    uint32_t charge_limit;
} SystemSettingsSceneChargingMenu;

static void
    system_settings_scene_charging_menu_limit_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SystemSettings* instance = context;
    SystemSettingsSceneChargingMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdChargingMenu);

    data->charge_limit = var_item_get_value(item);
    system_settings_send_custom_event(instance, SceneEventLimitChanged);
}

static void system_settings_scene_charging_menu_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SystemSettingsSceneChargingMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdChargingMenu);

    data->power = furi_record_open(RECORD_POWER);
    data->charge_limit = power_get_charge_limit(data->power);

    with_gui(instance->gui, {
        data->front = var_item_list_alloc(instance->front_scene_window);
        VarItem* front_limit = var_item_list_add_spinbox(
            data->front,
            "Limit (%)",
            NULL,
            60,
            100,
            5,
            system_settings_scene_charging_menu_limit_changed_callback,
            instance);
        var_item_set_value(front_limit, data->charge_limit);

        data->back = var_item_list_alloc(instance->back_scene_window);
        VarItem* back_limit =
            var_item_list_add_spinbox(data->back, "Limit (%)", NULL, 60, 100, 5, NULL, NULL);
        var_item_set_value(back_limit, data->charge_limit);

        widget_set_scrollbar_enabled(var_item_list_get_base(data->front), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(data->back), true);
    });
}

static void system_settings_scene_charging_menu_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    SystemSettingsSceneChargingMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdChargingMenu);

    with_gui(instance->gui, {
        var_item_list_free(data->front);
        var_item_list_free(data->back);
    });

    furi_record_close(RECORD_POWER);
}

static bool
    system_settings_scene_charging_menu_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        SystemSettingsSceneChargingMenu* data =
            scene_manager_get_scene_data(instance->scene_manager, SceneIdChargingMenu);

        switch(event->event) {
        case SceneEventLimitChanged:
            power_set_charge_limit(data->power, data->charge_limit);
            consumed = true;
            break;

        default:
            break;
        }
    }

    return consumed;
}

const Scene system_settings_scene_charging_menu = {
    .enter_callback = system_settings_scene_charging_menu_on_enter,
    .exit_callback = system_settings_scene_charging_menu_on_exit,
    .event_callback = system_settings_scene_charging_menu_on_event,
    .data_size = sizeof(SystemSettingsSceneChargingMenu),
};
