#include "../off_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#define S_TO_MS(s)  ((s) * 1000)
#define MS_TO_S(ms) ((ms) / 1000)

#define DELAY_MIN_SECONDS  5
#define DELAY_MAX_SECONDS  (24 * 60 * 60)
#define DELAY_STEP_SECONDS 5

typedef enum {
    SceneEventModeChanged = AppEventSceneEventsStart,
    SceneEventDelayChanged,
} SceneEvent;

typedef enum {
    VarItemListIdMode,
    VarItemListIdDelay,

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
} OffSettingsSceneMain;

static const char* sleep_mode_names[] = {
    [LowPowerModeSleepAfterHoldoff] = "Delayed",
    [LowPowerModeDisabled] = "Never",
};

static void scene_main_filter_items(OffSettingsSceneMain* data) {
    const bool show_delay = (data->settings.mode == LowPowerModeSleepAfterHoldoff);

    widget_set_visible((Widget*)data->front_container.items[VarItemListIdDelay], show_delay);
    widget_set_visible((Widget*)data->back_container.items[VarItemListIdDelay], show_delay);
}

static void scene_main_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    OffSettings* instance = context;
    OffSettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    data->settings.mode = var_item_get_value(item);
    scene_main_filter_items(data);
    off_settings_send_custom_event(instance, SceneEventModeChanged);
}

static void scene_main_delay_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    OffSettings* instance = context;
    OffSettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    data->settings.holdoff_duration = S_TO_MS(var_item_get_value(item));
    off_settings_send_custom_event(instance, SceneEventDelayChanged);
}

static void scene_main_fill_var_item_list(
    OffSettings* instance,
    VarItemListContainer* container,
    bool do_set_callbacks) {
    OffSettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    VarItem* mode_item = var_item_list_add_selector(
        container->list,
        "Shutdown",
        NULL,
        sleep_mode_names,
        COUNT_OF(sleep_mode_names),
        (do_set_callbacks) ? scene_main_mode_changed_callback : NULL,
        instance);

    var_item_set_value(mode_item, data->settings.mode);
    container->items[VarItemListIdMode] = mode_item;

    VarItem* delay_item = var_item_list_add_timebox_ex(
        container->list,
        "Delay",
        VarItemTimeboxBaseSeconds,
        DELAY_MIN_SECONDS,
        DELAY_MAX_SECONDS,
        DELAY_STEP_SECONDS,
        (do_set_callbacks) ? scene_main_delay_changed_callback : NULL,
        instance);

    var_item_set_value(delay_item, MS_TO_S(data->settings.holdoff_duration));
    container->items[VarItemListIdDelay] = delay_item;
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);

    OffSettings* instance = context;
    OffSettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    low_power_get_sleep_settings(instance->low_power, &data->settings);

    with_gui(instance->gui, {
        data->front_container.list = var_item_list_alloc(instance->front_scene_window);
        scene_main_fill_var_item_list(instance, &data->front_container, true);

        data->back_container.list = var_item_list_alloc(instance->back_scene_window);
        scene_main_fill_var_item_list(instance, &data->back_container, false);

        scene_main_filter_items(data);

        widget_set_scrollbar_enabled(var_item_list_get_base(data->front_container.list), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(data->back_container.list), true);
    });
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);

    OffSettings* instance = context;

    OffSettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        var_item_list_free(data->front_container.list);
        var_item_list_free(data->back_container.list);
    });
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    OffSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        OffSettingsSceneMain* data =
            scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

        switch(event->event) {
        case SceneEventModeChanged:
        case SceneEventDelayChanged:
            low_power_set_sleep_settings(instance->low_power, &data->settings);
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene off_settings_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(OffSettingsSceneMain),
};
