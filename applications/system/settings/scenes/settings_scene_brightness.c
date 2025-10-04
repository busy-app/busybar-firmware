#include "../settings.h"
#include "../models/brightness.h"

#include <gui/modules/var_item_list.h>

#define MANUAL_BRIGHTNESS_DEFAULT_VALUE 50

typedef enum {
    SceneCustomEventModeChanged = SettingsCustomEventSceneEventsStart,
    SceneCustomEventBrightnessChanged,
} SceneCustomEvent;

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

    _Atomic SettingsBrightnessMode mode;
    _Atomic uint8_t brightness;
} SettingsSceneBrightness;

static void settings_scene_brightness_filter_items(SettingsSceneBrightness* data) {
    const bool show_brightness = (data->mode == SettingsBrightnessModeManual);

    widget_set_visible(
        (Widget*)data->front_container.items[VarItemListIdBrightness], show_brightness);
    widget_set_visible(
        (Widget*)data->back_container.items[VarItemListIdBrightness], show_brightness);
}

static void settings_scene_brightness_mode_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->mode = var_item_get_value(item);
    settings_scene_brightness_filter_items(data);
    settings_send_custom_event(instance, SceneCustomEventModeChanged);
}

static void settings_scene_brightness_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->brightness = var_item_get_value(item);
    settings_send_custom_event(instance, SceneCustomEventBrightnessChanged);
}

static void settings_scene_brightness_fill_var_item_list(
    SettingsApp* instance,
    VarItemListContainer* container,
    bool do_set_callbacks) {
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    char brightness_mode_names[SettingsBrightnessModeCount][64] = {{0}, {0}};
    strcpy(
        brightness_mode_names[SettingsBrightnessModeManual],
        l10n_get(instance->l10n, L10N_KEY_SETTINGS_BRIGHTNESS_MODE_MANUAL));
    strcpy(
        brightness_mode_names[SettingsBrightnessModeAuto],
        l10n_get(instance->l10n, L10N_KEY_SETTINGS_BRIGHTNESS_MODE_AUTO));
    const char* brightness_modes[SettingsBrightnessModeCount] = {
        brightness_mode_names[0],
        brightness_mode_names[1],
    };

    VarItem* mode_item = var_item_list_add_selector(
        container->list,
        l10n_get(instance->l10n, L10N_KEY_SETTINGS_BRIGHTNESS_MODE),
        NULL,
        brightness_modes,
        SettingsBrightnessModeCount,
        (do_set_callbacks) ? settings_scene_brightness_mode_changed_callback : NULL,
        instance);

    var_item_set_value(mode_item, data->mode);

    VarItem* brightness_item = var_item_list_add_spinbox(
        container->list,
        l10n_get(instance->l10n, L10N_KEY_SETTINGS_BRIGHTNESS_LEVEL),
        "%",
        SETTINGS_BRIGHTNESS_RANGE_MIN,
        SETTINGS_BRIGHTNESS_RANGE_MAX,
        SETTINGS_BRIGHTNESS_STEP,
        (do_set_callbacks) ? settings_scene_brightness_changed_callback : NULL,
        instance);

    var_item_set_value(brightness_item, data->brightness);
    container->items[VarItemListIdBrightness] = brightness_item;
}

static void settings_scene_brightness_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    SettingsBrightnessMode mode = settings_brightness_get_mode(instance);
    data->mode = mode;
    data->brightness = (mode == SettingsBrightnessModeAuto) ? MANUAL_BRIGHTNESS_DEFAULT_VALUE :
                                                              settings_brightness_get(instance);

    with_gui(instance->gui, {
        data->front_container.list = var_item_list_alloc(instance->front_scene_window);
        settings_scene_brightness_fill_var_item_list(instance, &data->front_container, true);

        data->back_container.list = var_item_list_alloc(instance->back_scene_window);
        settings_scene_brightness_fill_var_item_list(instance, &data->back_container, false);

        settings_scene_brightness_filter_items(data);
    });
}

static void settings_scene_brightness_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        var_item_list_free(data->front_container.list);
        var_item_list_free(data->back_container.list);
    });
}

static bool settings_scene_brightness_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        SettingsSceneBrightness* data =
            scene_manager_get_current_scene_data(instance->scene_manager);

        switch(event->event) {
        case SceneCustomEventModeChanged:
            if(data->mode == SettingsBrightnessModeAuto) {
                settings_brightness_set_auto_mode(instance);
            } else {
                settings_brightness_set(instance, data->brightness);
            }
            consumed = true;
            break;

        case SceneCustomEventBrightnessChanged:
            settings_brightness_set(instance, data->brightness);
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(instance);
    }

    return consumed;
}

const Scene settings_scene_brightness = {
    .enter_callback = settings_scene_brightness_on_enter,
    .exit_callback = settings_scene_brightness_on_exit,
    .event_callback = settings_scene_brightness_on_event,
    .data_size = sizeof(SettingsSceneBrightness),
};
