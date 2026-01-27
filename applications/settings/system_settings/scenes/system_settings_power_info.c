#include "../system_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/label.h>

#define SYSTEM_SETTINGS_POWER_INFO_UPDATE_PERIOD_MS (500)

typedef enum {
    SceneEventPowerInfoStatusChangedEvent = AppEventSceneEventsStart,
} SceneEventPowerInfoEvent;

typedef struct {
    Label* power_info[GuiDisplayIdMax];
    FuriString* power_info_str;
} SettingsScenePowerInfo;

static void
    system_settings_power_info_update_data(SystemSettings* instance, FuriString* info_str) {
    PowerInfo info = {};
    power_get_info(instance->power, &info);

    furi_string_printf(info_str, "#888888 Battery level:# %u%%\n", info.charge);
    furi_string_cat_printf(
        info_str, "#888888 State:# %s\n", info.is_charging ? "Charging" : "Discharging");
    furi_string_cat_printf(
        info_str, "#888888 Battery volt:# %.2f\n", info.voltage_battery / 1000.f);
    furi_string_cat_printf(
        info_str, "#888888 Battery curr:# %.2f\n", info.current_battery / 1000.f);
    furi_string_cat_printf(
        info_str,
        "#888888 Battery temp:# %.1fC\n",
        power_get_temperature_battery_celsius(info.temperature_battery));
    furi_string_cat_printf(info_str, "#888888 USB volt:# %.2fV\n", info.voltage_usb / 1000.f);
    furi_string_cat_printf(info_str, "#888888 USB current:# %.2fA\n", info.current_usb / 1000.f);
}

static void system_settings_power_info_update(void* context) {
    furi_check(context);

    SystemSettings* instance = context;
    SettingsScenePowerInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerInfo);

    system_settings_power_info_update_data(instance, scene->power_info_str);
    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            label_set_text(scene->power_info[disp], furi_string_get_cstr(scene->power_info_str));
        }
    });
}

static void system_settings_scene_power_info_on_enter(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;

    SettingsScenePowerInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerInfo);
    scene->power_info_str = furi_string_alloc();

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    with_gui(instance->gui, {
        nav_bar_push_location(instance->back_nav_bar, "POWER INFO");
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            widget_set_scrollbar_mode(windows[disp], WidgetScrollBarModeAuto);
            scene->power_info[disp] = label_alloc(windows[disp]);
            lable_set_inline_text_color_formatting(scene->power_info[disp], true);
            system_settings_power_info_update_data(instance, scene->power_info_str);
            label_set_text(scene->power_info[disp], furi_string_get_cstr(scene->power_info_str));
        }
    });

    furi_event_loop_tick_set(
        instance->event_loop,
        SYSTEM_SETTINGS_POWER_INFO_UPDATE_PERIOD_MS,
        system_settings_power_info_update,
        instance);
}

static void system_settings_scene_power_info_on_exit(void* context) {
    furi_assert(context);
    SystemSettings* instance = context;
    SettingsScenePowerInfo* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdPowerInfo);

    furi_string_free(scene->power_info_str);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->back_nav_bar);

        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            label_free(scene->power_info[disp]);
        }
        furi_event_loop_tick_set(instance->event_loop, 0, NULL, NULL);
    });
}

static bool
    system_settings_scene_power_info_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    bool consumed = false;
    UNUSED(event);

    return consumed;
}

const Scene system_settings_scene_power_info = {
    .enter_callback = system_settings_scene_power_info_on_enter,
    .exit_callback = system_settings_scene_power_info_on_exit,
    .event_callback = system_settings_scene_power_info_on_event,
    .data_size = sizeof(SettingsScenePowerInfo),
};
