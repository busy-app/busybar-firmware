#include "../settings.h"

#include <gui/modules/var_item_list.h>

typedef enum {
    BrightnessModeManual,
    BrightnessModeAuto,

    BrightnessModesCount,
} BrightnessMode;

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

    BrightnessMode mode;
    uint8_t brightness;
} SettingsSceneBrightness;

static const char* brightness_mode_names[BrightnessModesCount] = {
    [BrightnessModeManual] = "Manual",
    [BrightnessModeAuto] = "Auto",
};

static void settings_scene_brightness_filter_items(SettingsSceneBrightness* data) {
    const bool show_brightness = (data->mode == BrightnessModeManual);

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
    settings_send_custom_event(
        instance,
        (data->mode == BrightnessModeAuto) ? BACK_DISPLAY_BRIGHTNESS_AUTO : data->brightness);
}

static void settings_scene_brightness_changed_callback(VarItem* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->brightness = var_item_get_value(item);
    settings_send_custom_event(instance, data->brightness);
}

static void settings_scene_brightness_fill_var_item_list(
    SettingsApp* instance,
    VarItemListContainer* container,
    bool do_set_callbacks) {
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    VarItem* mode_item = var_item_list_add_selector(
        container->list,
        "Mode",
        NULL,
        brightness_mode_names,
        BrightnessModesCount,
        (do_set_callbacks) ? settings_scene_brightness_mode_changed_callback : NULL,
        instance);

    var_item_set_value(mode_item, data->mode);

    VarItem* brightness_item = var_item_list_add_spinbox(
        container->list,
        "Level",
        "%",
        5,
        100,
        5,
        (do_set_callbacks) ? settings_scene_brightness_changed_callback : NULL,
        instance);

    var_item_set_value(brightness_item, data->brightness);
    container->items[VarItemListIdBrightness] = brightness_item;
}

static void settings_scene_brightness_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneBrightness* data = scene_manager_get_current_scene_data(instance->scene_manager);

    uint8_t brightness = back_display_get_brightness(instance->back_display);
    if(brightness == BACK_DISPLAY_BRIGHTNESS_AUTO) {
        data->mode = BrightnessModeAuto;
        data->brightness = 50;
    } else {
        data->mode = BrightnessModeManual;
        data->brightness = brightness;
    }

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
        front_display_set_brightness(instance->front_display, event->event);
        back_display_set_brightness(instance->back_display, event->event);

        consumed = true;
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
