#include "../busy.h"

#include <gui/modules/var_item_list.h>
#include <gui/modules/anim_image.h>

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;
    BusyTimerConfig timer_config;
} BusySceneSetupTimer;

static void busy_scene_setup_timer_work_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.work_time_mn = var_item_get_value(item);
}

static void busy_scene_setup_timer_rest_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.rest_time_mn = var_item_get_value(item);
}

static void busy_scene_setup_timer_cycles_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.cycle_count = var_item_get_value(item);
}

static void busy_scene_setup_timer_autostart_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.enable_autostart = var_item_get_value(item);
}

static void busy_scene_setup_timer_speed_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.enable_speed = var_item_get_value(item);
}

static void
    busy_scene_setup_fill_var_item_list(VarItemList* list, BusySceneSetupTimer* data, bool set_cb) {
    VarItem* item;

    item = var_item_list_add_timebox(
        list,
        "Work",
        BUSY_TIMER_WORK_TIME_MIN_MN,
        BUSY_TIMER_WORK_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        set_cb ? busy_scene_setup_timer_work_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.work_time_mn);

    item = var_item_list_add_timebox(
        list,
        "Rest",
        BUSY_TIMER_REST_TIME_MIN_MN,
        BUSY_TIMER_REST_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        set_cb ? busy_scene_setup_timer_rest_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.rest_time_mn);

    item = var_item_list_add_spinbox(
        list,
        "Cycles",
        NULL,
        BUSY_TIMER_CYCLE_COUNT_MIN,
        BUSY_TIMER_CYCLE_COUNT_MAX,
        BUSY_TIMER_CYCLE_INCREMENT,
        set_cb ? busy_scene_setup_timer_cycles_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.cycle_count);

    item = var_item_list_add_switch(
        list, "Autostart", set_cb ? busy_scene_setup_timer_autostart_changed_callback : NULL, data);

    var_item_set_value(item, data->timer_config.enable_autostart);

    item = var_item_list_add_switch(
        list, "Fast mode", set_cb ? busy_scene_setup_timer_speed_changed_callback : NULL, data);

    var_item_set_value(item, data->timer_config.enable_speed);
}

static void busy_scene_setup_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    busy_timer_get_config(instance->busy_timer, &data->timer_config);

    with_gui(instance->gui, {
        data->front_list = var_item_list_alloc(instance->front_window);
        busy_scene_setup_fill_var_item_list(data->front_list, data, true);

        data->back_list = var_item_list_alloc(nav_stack_get_base(instance->nav_stack));
        // TODO: Fix the layout to set appropriate sizes for children
        widget_set_height(var_item_list_get_base(data->back_list), 58);
        busy_scene_setup_fill_var_item_list(data->back_list, data, false);
    });
}

static void busy_scene_setup_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    busy_timer_set_config(instance->busy_timer, &data->timer_config);

    with_gui(instance->gui, {
        var_item_list_free(data->front_list);
        var_item_list_free(data->back_list);
    });
}

static bool busy_scene_setup_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    BusyApp* instance = context;

    if(event->type == SceneManagerEventTypeBack) {
        busy_pop_location(instance);
    }

    return consumed;
}

const Scene busy_scene_setup_timer = {
    .enter_callback = busy_scene_setup_timer_on_enter,
    .exit_callback = busy_scene_setup_timer_on_exit,
    .event_callback = busy_scene_setup_timer_on_event,
    .data_size = sizeof(BusySceneSetupTimer),
};
