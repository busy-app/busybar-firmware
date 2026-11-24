#include "../../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#include <furi_hal_nvm.h>

#define MIN_TO_MS(s)  ((s) * 60 * 1000)
#define MS_TO_MIN(ms) ((ms) / 1000 / 60)

#define DELAY_MIN_MINUTES                  5
#define DELAY_MIN_MINUTES_IF_DEBUG_ENABLED 0
#define DELAY_MAX_MINUTES                  (4 * 60)
#define DELAY_STEP_MINUTES                 5

typedef enum {
    SceneEventModeChanged = AppEventSceneEventsStart,
    SceneEventDelayChanged,
    SceneEventLearnMore,
} SceneEvent;

typedef enum {
    VarItemListIdMode,
    VarItemListIdDelay,
    VarItemListIdInformation,

    VarItemListIdsCount,
} VarItemListId;

typedef struct {
    VarItemList* list;
    VarItem* items[VarItemListIdsCount];
} VarItemListContainer;

typedef struct {
    VarItemListContainer front_container;
    VarItemListContainer back_container;
    LowPowerSleepSettings settings;
} SystemSettingsSceneOffBehaviorMenu;

static const char* sleep_mode_names[] = {
    [LowPowerModeSleepAfterHoldoff] = "Turn off",
    [LowPowerModeDisabled] = "Standby",
};

static void system_settings_scene_off_behavior_menu_filter_items(
    SystemSettingsSceneOffBehaviorMenu* data) {
    const bool show_delay = (data->settings.mode == LowPowerModeSleepAfterHoldoff);

    widget_set_visible((Widget*)data->front_container.items[VarItemListIdDelay], show_delay);
    widget_set_visible((Widget*)data->back_container.items[VarItemListIdDelay], show_delay);
}

static void
    system_settings_scene_off_behavior_menu_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SystemSettings* instance = context;
    SystemSettingsSceneOffBehaviorMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehavior);

    data->settings.mode = var_item_get_value(item);
    system_settings_scene_off_behavior_menu_filter_items(data);
    system_settings_send_custom_event(instance, SceneEventModeChanged);
}

static void
    system_settings_scene_off_behavior_menu_delay_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SystemSettings* instance = context;
    SystemSettingsSceneOffBehaviorMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehavior);

    data->settings.holdoff_duration = MIN_TO_MS(var_item_get_value(item));
    system_settings_send_custom_event(instance, SceneEventDelayChanged);
}

static void
    system_settings_scene_off_behavior_menu_lean_more_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SystemSettings* instance = context;
    system_settings_send_custom_event(instance, SceneEventLearnMore);
}

static void system_settings_scene_off_behavior_menu_fill_var_item_list(
    SystemSettings* instance,
    VarItemListContainer* container,
    bool do_set_callbacks) {
    SystemSettingsSceneOffBehaviorMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehavior);

    VarItem* mode_item = var_item_list_add_selector(
        container->list,
        "Mode",
        NULL,
        sleep_mode_names,
        COUNT_OF(sleep_mode_names),
        (do_set_callbacks) ? system_settings_scene_off_behavior_menu_mode_changed_callback : NULL,
        instance);

    var_item_set_value(mode_item, data->settings.mode);
    container->items[VarItemListIdMode] = mode_item;

    int32_t min_minutes = furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug) ?
                              DELAY_MIN_MINUTES_IF_DEBUG_ENABLED :
                              DELAY_MIN_MINUTES;

    VarItem* delay_item = var_item_list_add_timebox(
        container->list,
        "Turn off in",
        min_minutes,
        DELAY_MAX_MINUTES,
        DELAY_STEP_MINUTES,
        (do_set_callbacks) ? system_settings_scene_off_behavior_menu_delay_changed_callback : NULL,
        instance);

    var_item_set_value(delay_item, MS_TO_MIN(data->settings.holdoff_duration));
    container->items[VarItemListIdDelay] = delay_item;

    VarItem* info_item = var_item_list_add_button(
        container->list,
        "Learn more",
        system_settings_scene_off_behavior_menu_lean_more_callback,
        instance);
    container->items[VarItemListIdInformation] = info_item;
}

static void system_settings_scene_off_behavior_menu_on_enter(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;
    SystemSettingsSceneOffBehaviorMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehavior);

    low_power_get_sleep_settings(instance->low_power, &data->settings);

    with_gui(instance->gui, {
        data->front_container.list = var_item_list_alloc(instance->front_scene_window);
        system_settings_scene_off_behavior_menu_fill_var_item_list(
            instance, &data->front_container, true);

        data->back_container.list = var_item_list_alloc(instance->back_scene_window);
        system_settings_scene_off_behavior_menu_fill_var_item_list(
            instance, &data->back_container, false);

        system_settings_scene_off_behavior_menu_filter_items(data);

        widget_set_scrollbar_enabled(var_item_list_get_base(data->front_container.list), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(data->back_container.list), true);
    });
}

static void system_settings_scene_off_behavior_menu_on_exit(void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    SystemSettingsSceneOffBehaviorMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehavior);

    with_gui(instance->gui, {
        var_item_list_free(data->front_container.list);
        var_item_list_free(data->back_container.list);
    });
}

static bool system_settings_scene_off_behavior_menu_on_event(
    const SceneManagerEvent* event,
    void* context) {
    furi_assert(context);

    SystemSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        SystemSettingsSceneOffBehaviorMenu* data =
            scene_manager_get_scene_data(instance->scene_manager, SceneIdOffBehavior);

        switch(event->event) {
        case SceneEventModeChanged:
        case SceneEventDelayChanged:
            low_power_set_sleep_settings(instance->low_power, &data->settings);
            consumed = true;
            break;

        case SceneEventLearnMore:
            scene_manager_next_scene(instance->scene_manager, SceneIdOffBehaviorInformation);
            break;

        default:
            break;
        }
    }

    return consumed;
}

const Scene system_settings_scene_off_behavior_menu = {
    .enter_callback = system_settings_scene_off_behavior_menu_on_enter,
    .exit_callback = system_settings_scene_off_behavior_menu_on_exit,
    .event_callback = system_settings_scene_off_behavior_menu_on_event,
    .data_size = sizeof(SystemSettingsSceneOffBehaviorMenu),
};
