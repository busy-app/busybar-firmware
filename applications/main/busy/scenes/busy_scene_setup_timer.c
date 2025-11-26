#include "../busy_i.h"

#include <gui/modules/var_item_list.h>

typedef enum {
    // Timer Mode is not included because it is always shown
    VarItemListIdTime,
    VarItemListIdWork,
    VarItemListIdRest,
    VarItemListIdCycles,
    VarItemListIdAutostart,
    // Demo Mode is not included because it is always shown
    VarItemListIdMax,
} VarItemListId;

typedef struct {
    VarItemList* list;
    VarItem* items[VarItemListIdMax];
} VarItemListContainer;

typedef struct {
    VarItemListContainer containers[GuiDisplayIdMax];
    BusyTimerConfig timer_config;
} BusySceneSetupTimer;

static void busy_scene_setup_timer_filter_items(BusySceneSetupTimer* data) {
    // Which items to show w/ respect to the current timer mode
    static const bool is_shown_table[BusyTimerModeMax][VarItemListIdMax] = {
        [BusyTimerModeInfinite] = {0},
        [BusyTimerModeSimple] = {true, /* filled with zeroes */},
        [BusyTimerModeInterval] = {false, true, true, true, true},
    };

    const BusyTimerMode timer_mode = data->timer_config.mode;
    const bool* const is_shown_in_mode = is_shown_table[timer_mode];

    for(GuiDisplayId display_id = 0; display_id < GuiDisplayIdMax; ++display_id) {
        VarItemListContainer* container = &data->containers[display_id];
        VarItem** items = container->items;

        for(VarItemListId item_id = 0; item_id < VarItemListIdMax; ++item_id) {
            widget_set_visible((Widget*)items[item_id], is_shown_in_mode[item_id]);
        }
    }
}

static void busy_scene_setup_timer_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.mode = var_item_get_value(item);

    busy_scene_setup_timer_filter_items(data);
}

static void busy_scene_setup_timer_time_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.time_mn = var_item_get_value(item);
}

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

static void busy_scene_setup_timer_demo_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    data->timer_config.enable_demo_mode = var_item_get_value(item);
}

static void
    busy_scene_setup_fill_var_item_list(BusySceneSetupTimer* data, GuiDisplayId display_id) {
    const bool set_cb = (display_id == GuiDisplayIdFront);

    VarItemListContainer* container = &data->containers[display_id];
    VarItemListId item_id = 0;
    VarItem* item;

    item = var_item_list_add_selector(
        container->list,
        "Mode",
        NULL,
        busy_timer_get_mode_names(),
        BusyTimerModeMax,
        set_cb ? busy_scene_setup_timer_mode_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.mode);

    // IMPORTANT: NOT storing the first item because it is always shown

    item = var_item_list_add_timebox(
        container->list,
        "Time",
        BUSY_TIMER_TIME_MIN_MN,
        BUSY_TIMER_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        set_cb ? busy_scene_setup_timer_time_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.time_mn);

    container->items[item_id++] = item;

    item = var_item_list_add_timebox(
        container->list,
        "Work",
        BUSY_TIMER_WORK_TIME_MIN_MN,
        BUSY_TIMER_WORK_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        set_cb ? busy_scene_setup_timer_work_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.work_time_mn);

    container->items[item_id++] = item;

    item = var_item_list_add_timebox(
        container->list,
        "Rest",
        BUSY_TIMER_REST_TIME_MIN_MN,
        BUSY_TIMER_REST_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        set_cb ? busy_scene_setup_timer_rest_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.rest_time_mn);

    container->items[item_id++] = item;

    item = var_item_list_add_spinbox(
        container->list,
        "Cycles",
        NULL,
        BUSY_TIMER_CYCLE_COUNT_MIN,
        BUSY_TIMER_CYCLE_COUNT_MAX,
        BUSY_TIMER_CYCLE_INCREMENT,
        set_cb ? busy_scene_setup_timer_cycles_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.cycle_count);

    container->items[item_id++] = item;

    item = var_item_list_add_switch(
        container->list,
        "Autostart",
        set_cb ? busy_scene_setup_timer_autostart_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.enable_autostart);

    container->items[item_id++] = item;

    item = var_item_list_add_switch(
        container->list,
        "Demo mode",
        set_cb ? busy_scene_setup_timer_demo_mode_changed_callback : NULL,
        data);

    var_item_set_value(item, data->timer_config.enable_demo_mode);

    // IMPORTANT: NOT storing the last item because it is always shown
}

static void busy_scene_setup_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTimer);

    busy_timer_get_config(instance->busy_timer, &data->timer_config);

    with_gui(instance->gui, {
        data->containers[GuiDisplayIdFront].list = var_item_list_alloc(instance->front_window);
        data->containers[GuiDisplayIdBack].list = var_item_list_alloc(instance->back_window);

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            busy_scene_setup_fill_var_item_list(data, id);
        }

        busy_scene_setup_timer_filter_items(data);
    });
}

static void busy_scene_setup_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTimer);

    busy_timer_set_config(instance->busy_timer, &data->timer_config);

    with_gui(instance->gui, {
        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            var_item_list_free(data->containers[id].list);
        }
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
