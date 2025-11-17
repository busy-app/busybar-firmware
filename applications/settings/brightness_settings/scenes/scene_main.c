#include "../brightness_settings.h"
#include "../models/brightness.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#define MANUAL_BRIGHTNESS_DEFAULT_VALUE 50

typedef enum {
    SceneEventModeChanged = AppEventSceneEventsStart,
    SceneEventBrightnessChanged,
} SceneEvent;

typedef enum {
    VarItemListIdBrightness,

    VarItemListIdsCount,
} VarItemListId;

typedef struct {
    VarItemList* list;
    VarItem* items[VarItemListIdsCount];
} VarItemListContainer;

typedef struct {
    VarItemListContainer front_container;
    VarItemListContainer back_container;

    _Atomic BrightnessMode mode;
    _Atomic uint8_t brightness;
} SettingsSceneBrightness;

static const char* brightness_mode_names[] = {
    [BrightnessModeManual] = "Manual",
    [BrightnessModeAuto] = "Auto",
};

static_assert(COUNT_OF(brightness_mode_names) == BrightnessModesCount);

static void scene_main_filter_items(SettingsSceneBrightness* data) {
    const bool show_brightness = (data->mode == BrightnessModeManual);

    widget_set_visible(
        (Widget*)data->front_container.items[VarItemListIdBrightness], show_brightness);
    widget_set_visible(
        (Widget*)data->back_container.items[VarItemListIdBrightness], show_brightness);
}

static void scene_main_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BrightnessSettings* instance = context;
    SettingsSceneBrightness* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    data->mode = var_item_get_value(item);
    scene_main_filter_items(data);
    brightness_settings_send_custom_event(instance, SceneEventModeChanged);
}

static void scene_main_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BrightnessSettings* instance = context;
    SettingsSceneBrightness* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    data->brightness = var_item_get_value(item);
    brightness_settings_send_custom_event(instance, SceneEventBrightnessChanged);
}

static void scene_main_fill_var_item_list(
    BrightnessSettings* instance,
    VarItemListContainer* container,
    bool do_set_callbacks) {
    SettingsSceneBrightness* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    VarItem* mode_item = var_item_list_add_selector(
        container->list,
        "Mode",
        NULL,
        brightness_mode_names,
        COUNT_OF(brightness_mode_names),
        (do_set_callbacks) ? scene_main_mode_changed_callback : NULL,
        instance);

    var_item_set_value(mode_item, data->mode);

    VarItem* brightness_item = var_item_list_add_spinbox(
        container->list,
        "Level",
        "%",
        BRIGHTNESS_RANGE_MIN,
        BRIGHTNESS_RANGE_MAX,
        BRIGHTNESS_STEP,
        (do_set_callbacks) ? scene_main_changed_callback : NULL,
        instance);

    var_item_set_value(brightness_item, data->brightness);
    container->items[VarItemListIdBrightness] = brightness_item;
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);

    BrightnessSettings* instance = context;
    SettingsSceneBrightness* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    BrightnessMode mode = brightness_model_get_mode(instance->model);
    data->mode = mode;
    data->brightness = (mode == BrightnessModeAuto) ? MANUAL_BRIGHTNESS_DEFAULT_VALUE :
                                                      brightness_model_get(instance->model);

    with_gui(instance->gui, {
        data->front_container.list = var_item_list_alloc(instance->front_scene_window);
        scene_main_fill_var_item_list(instance, &data->front_container, true);

        data->back_container.list = var_item_list_alloc(instance->back_scene_window);
        scene_main_fill_var_item_list(instance, &data->back_container, false);

        scene_main_filter_items(data);
    });

    Color color = COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF);
    status_lights_run_preset(instance->status_lights, StatusLightsPresetStaticColor, color);
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);

    BrightnessSettings* instance = context;
    SettingsSceneBrightness* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        var_item_list_free(data->front_container.list);
        var_item_list_free(data->back_container.list);
    });

    status_lights_run_preset(instance->status_lights, StatusLightsPresetOff, (Color){});
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BrightnessSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        SettingsSceneBrightness* data =
            scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

        switch(event->event) {
        case SceneEventModeChanged:
            if(data->mode == BrightnessModeAuto) {
                brightness_model_set_auto_mode(instance->model);
            } else {
                brightness_model_set(instance->model, data->brightness);
            }
            consumed = true;
            break;

        case SceneEventBrightnessChanged:
            brightness_model_set(instance->model, data->brightness);
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

const Scene brightness_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(SettingsSceneBrightness),
};
