#include "../busy_i.h"

#include <furi_hal_nvm.h>

#include <gui/modules/var_item_list.h>

typedef enum {
    VarItemListIdMode,
    VarItemListIdTime,
    VarItemListIdWork,
    VarItemListIdRest,
    VarItemListIdCycles,
    VarItemListIdAutostart,
    VarItemListIdShowWork,
    VarItemListIdShowWorkTime,
    VarItemListIdDemoMode,
    VarItemListIdMax,
} VarItemListId;

typedef struct {
    VarItemList* list;
    VarItem* items[VarItemListIdMax];
} VarItemListContainer;

typedef struct {
    VarItemListContainer containers[GuiDisplayIdMax];
} BusySceneSetupTimer;

static void busy_scene_setup_timer_set_item_defaults(const BusySceneSetupTimer* data) {
    static const int32_t default_values_table[] = {
        // VarItemListIdMode is not reset to default
        [VarItemListIdTime] = BUSY_TIMER_TIME_DEFAULT_MN,
        [VarItemListIdWork] = BUSY_TIMER_WORK_TIME_DEFAULT_MN,
        [VarItemListIdRest] = BUSY_TIMER_REST_TIME_DEFAULT_MN,
        [VarItemListIdCycles] = BUSY_TIMER_CYCLE_COUNT_DEFAULT,
        [VarItemListIdAutostart] = BUSY_TIMER_ENABLE_AUTOSTART_DEFAULT,
        // VarItemListIdShowWork and VarItemListIdDemoMode are not reset to default
    };

    for(GuiDisplayId display_id = 0; display_id < GuiDisplayIdMax; ++display_id) {
        const VarItemListContainer* container = &data->containers[display_id];
        VarItem* const* items = container->items;

        // NOTE: Starting from the second element
        for(VarItemListId item_id = VarItemListIdTime; item_id < COUNT_OF(default_values_table);
            ++item_id) {
            var_item_set_value(items[item_id], default_values_table[item_id]);
        }
    }
}

static void busy_scene_setup_timer_filter_items(BusySceneSetupTimer* data) {
    // Which items to show w/ respect to the current timer mode
    static const bool is_shown_table[BusyTimerModeMax][VarItemListIdMax] = {
        [BusyTimerModeInfinite] =
            {
                [VarItemListIdMode] = true,
                [VarItemListIdShowWork] = true,
                [VarItemListIdDemoMode] = true,
            },
        [BusyTimerModeSimple] =
            {
                [VarItemListIdMode] = true,
                [VarItemListIdTime] = true,
                [VarItemListIdShowWork] = true,
                [VarItemListIdShowWorkTime] = true,
                [VarItemListIdDemoMode] = true,
            },
        [BusyTimerModeInterval] =
            {
                [VarItemListIdMode] = true,
                [VarItemListIdWork] = true,
                [VarItemListIdRest] = true,
                [VarItemListIdCycles] = true,
                [VarItemListIdAutostart] = true,
                [VarItemListIdShowWork] = true,
                [VarItemListIdShowWorkTime] = true,
                [VarItemListIdDemoMode] = true,
            },
    };

    const VarItemListContainer* containers = data->containers;

    const VarItem* mode_item = containers[GuiDisplayIdFront].items[VarItemListIdMode];
    const BusyTimerMode timer_mode = var_item_get_value(mode_item);
    const bool* const is_shown_in_mode = is_shown_table[timer_mode];

    for(GuiDisplayId display_id = 0; display_id < GuiDisplayIdMax; ++display_id) {
        const VarItemListContainer* container = &containers[display_id];
        VarItem* const* items = container->items;

        for(VarItemListId item_id = 0; item_id < VarItemListIdMax; ++item_id) {
            VarItem* item = items[item_id];
            widget_set_visible((Widget*)item, is_shown_in_mode[item_id]);
        }

        if(!furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
            widget_set_visible((Widget*)items[VarItemListIdDemoMode], false);
        }
    }
}

static void busy_scene_setup_timer_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusySceneSetupTimer* data = context;
    busy_scene_setup_timer_filter_items(data);
    busy_scene_setup_timer_set_item_defaults(data);
}

static void
    busy_scene_setup_fill_var_item_list(BusySceneSetupTimer* data, GuiDisplayId display_id) {
    const bool set_cb = (display_id == GuiDisplayIdFront);

    VarItemListContainer* container = &data->containers[display_id];
    VarItem** items = container->items;
    VarItemListId item_id = 0;

    items[item_id++] = var_item_list_add_selector(
        container->list,
        "Mode",
        NULL,
        busy_timer_get_mode_names(),
        BusyTimerModeMax,
        set_cb ? busy_scene_setup_timer_mode_changed_callback : NULL,
        data);

    items[item_id++] = var_item_list_add_timebox(
        container->list,
        "Time",
        BUSY_TIMER_TIME_MIN_MN,
        BUSY_TIMER_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        NULL,
        NULL);

    items[item_id++] = var_item_list_add_timebox(
        container->list,
        "Work",
        BUSY_TIMER_WORK_TIME_MIN_MN,
        BUSY_TIMER_WORK_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        NULL,
        NULL);

    items[item_id++] = var_item_list_add_timebox(
        container->list,
        "Rest",
        BUSY_TIMER_REST_TIME_MIN_MN,
        BUSY_TIMER_REST_TIME_MAX_MN,
        BUSY_TIMER_TIME_INCREMENT_MN,
        NULL,
        NULL);

    items[item_id++] = var_item_list_add_spinbox(
        container->list,
        "Cycles",
        NULL,
        BUSY_TIMER_CYCLE_COUNT_MIN,
        BUSY_TIMER_CYCLE_COUNT_MAX,
        BUSY_TIMER_CYCLE_INCREMENT,
        NULL,
        NULL);

    items[item_id++] = var_item_list_add_switch(container->list, "Autostart", NULL, NULL);

    items[item_id++] =
        var_item_list_add_switch(container->list, "Show work\nphase only", NULL, NULL);

    items[item_id++] = var_item_list_add_switch(container->list, "Show work\ntime", NULL, NULL);

    items[item_id++] = var_item_list_add_switch(container->list, "Demo mode", NULL, NULL);
}

static void busy_scene_setup_init_var_item_values(
    const BusySceneSetupTimer* data,
    GuiDisplayId display_id,
    const BusyTimerPreset* timer_preset) {
    const VarItemListContainer* container = &data->containers[display_id];
    VarItem* const* items = container->items;

    const BusyAppConfig* app_config = &timer_preset->app_config;
    const BusyTimerConfig* timer_config = &timer_preset->timer_config;

    var_item_set_value(items[VarItemListIdMode], timer_config->mode);

    if(timer_config->mode == BusyTimerModeSimple) {
        const BusyTimerSimpleConfig* simple_config = &timer_config->simple;
        var_item_set_value(items[VarItemListIdTime], MS_TO_M(simple_config->total_time_ms));

    } else if(timer_config->mode == BusyTimerModeInterval) {
        const BusyTimerIntervalConfig* interval_config = &timer_config->interval;
        var_item_set_value(items[VarItemListIdWork], MS_TO_M(interval_config->work_time_ms));
        var_item_set_value(items[VarItemListIdRest], MS_TO_M(interval_config->rest_time_ms));
        var_item_set_value(items[VarItemListIdCycles], interval_config->cycles_count);
        var_item_set_value(items[VarItemListIdAutostart], interval_config->is_autostart_enabled);
    }

    var_item_set_value(items[VarItemListIdShowWork], app_config->is_show_work_only_enabled);
    var_item_set_value(items[VarItemListIdShowWorkTime], app_config->is_show_work_time_enabled);
    var_item_set_value(items[VarItemListIdDemoMode], timer_preset->is_demo_mode_enabled);
}

static void busy_scene_setup_get_var_item_values(
    const BusySceneSetupTimer* data,
    BusyTimerPreset* timer_preset) {
    VarItem* const* items = data->containers[GuiDisplayIdFront].items;

    BusyAppConfig* app_config = &timer_preset->app_config;
    BusyTimerConfig* timer_config = &timer_preset->timer_config;

    timer_config->mode = var_item_get_value(items[VarItemListIdMode]);

    if(timer_config->mode == BusyTimerModeSimple) {
        BusyTimerSimpleConfig* simple_config = &timer_config->simple;
        simple_config->total_time_ms = M_TO_MS(var_item_get_value(items[VarItemListIdTime]));

    } else if(timer_config->mode == BusyTimerModeInterval) {
        BusyTimerIntervalConfig* interval_config = &timer_config->interval;
        interval_config->work_time_ms = M_TO_MS(var_item_get_value(items[VarItemListIdWork]));
        interval_config->rest_time_ms = M_TO_MS(var_item_get_value(items[VarItemListIdRest]));
        interval_config->cycles_count = var_item_get_value(items[VarItemListIdCycles]);
        interval_config->is_autostart_enabled = var_item_get_value(items[VarItemListIdAutostart]);
    }

    app_config->is_show_work_only_enabled = var_item_get_value(items[VarItemListIdShowWork]);
    app_config->is_show_work_time_enabled = var_item_get_value(items[VarItemListIdShowWorkTime]);
    timer_preset->is_demo_mode_enabled = var_item_get_value(items[VarItemListIdDemoMode]);
}

static void
    busy_scene_setup_update_timer_preset(BusyApp* instance, const BusySceneSetupTimer* data) {
    BusyTimerPreset timer_preset;

    busy_get_timer_preset(instance, &timer_preset);
    busy_scene_setup_get_var_item_values(data, &timer_preset);
    busy_set_timer_preset(instance, &timer_preset);
}

static void busy_scene_setup_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTimer);

    BusyTimerPreset timer_preset;
    busy_get_timer_preset(instance, &timer_preset);

    with_gui(instance->gui, {
        data->containers[GuiDisplayIdFront].list = var_item_list_alloc(instance->front_window);
        data->containers[GuiDisplayIdBack].list = var_item_list_alloc(instance->back_window);

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            busy_scene_setup_fill_var_item_list(data, id);
            busy_scene_setup_init_var_item_values(data, id, &timer_preset);
        }

        busy_scene_setup_timer_filter_items(data);

        widget_set_scrollbar_enabled(
            var_item_list_get_base(data->containers[GuiDisplayIdFront].list), true);
        widget_set_scrollbar_enabled(
            var_item_list_get_base(data->containers[GuiDisplayIdBack].list), true);
    });
}

static void busy_scene_setup_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTimer);

    if(!instance->show_timer_requested) {
        // NOTE: Not saving timer preset if launched via snapshot to avoid deadlocks
        busy_scene_setup_update_timer_preset(instance, data);
    }

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
