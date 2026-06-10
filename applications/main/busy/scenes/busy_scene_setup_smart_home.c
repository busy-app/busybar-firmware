#include "../busy_i.h"

#include <gui/modules/var_item_list.h>

#define ITEM_LABEL_ENABLE "Trigger smart\nhome"

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;
    VarItem* saved_item;
} BusySceneSetupSmartHome;

static void busy_scene_setup_smart_home_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupSmartHome* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupSmartHome);

    BusyTimerPreset timer_preset;
    busy_get_timer_preset(instance, &timer_preset);

    const bool is_smart_home_enabled = timer_preset.app_config.is_smart_home_enabled;

    with_gui(instance->gui, {
        data->front_list = var_item_list_alloc(instance->front_window);
        data->back_list = var_item_list_alloc(instance->back_window);

        VarItem* item;

        item = var_item_list_add_switch(data->front_list, ITEM_LABEL_ENABLE, NULL, NULL);
        var_item_set_value(item, is_smart_home_enabled);

        item = var_item_list_add_switch(data->back_list, ITEM_LABEL_ENABLE, NULL, NULL);
        var_item_set_value(item, is_smart_home_enabled);
        // Saving the last item to get its value later
        data->saved_item = item;

        widget_set_scrollbar_enabled(var_item_list_get_base(data->front_list), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(data->back_list), true);
    });
}

static void busy_scene_setup_smart_home_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupSmartHome* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupSmartHome);

    BusyTimerPreset timer_preset;
    busy_get_timer_preset(instance, &timer_preset);

    BusyAppConfig* app_config = &timer_preset.app_config;
    app_config->is_smart_home_enabled = var_item_get_value(data->saved_item);

    instance->config = *app_config;
    busy_set_timer_preset(instance, &timer_preset);

    with_gui(instance->gui, {
        var_item_list_free(data->front_list);
        var_item_list_free(data->back_list);
    });
}

static bool busy_scene_setup_smart_home_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    BusyApp* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_pop_location(instance);
    }

    return consumed;
}

const Scene busy_scene_setup_smart_home = {
    .enter_callback = busy_scene_setup_smart_home_on_enter,
    .exit_callback = busy_scene_setup_smart_home_on_exit,
    .event_callback = busy_scene_setup_smart_home_on_event,
    .data_size = sizeof(BusySceneSetupSmartHome),
};
