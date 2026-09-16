#include "system_settings_scenes.h"

extern const Scene system_settings_scene_main;
extern const Scene system_settings_scene_power_menu;
extern const Scene system_settings_scene_power_shut_down_confirm;
extern const Scene system_settings_scene_power_unplug_usb;
extern const Scene system_settings_scene_power_restart_confirm;
extern const Scene system_settings_scene_power_restart;
extern const Scene system_settings_scene_power_info;
extern const Scene system_settings_scene_debug;
extern const Scene system_settings_scene_telemetry;
extern const Scene system_settings_scene_factory_reset_confirm;
extern const Scene system_settings_scene_factory_reset;
extern const Scene system_settings_internal_scene_low_battery;

const Scene* const system_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &system_settings_scene_main,

    [SceneIdPowerMenu] = &system_settings_scene_power_menu,
    [SceneIdPowerShutDownConfirm] = &system_settings_scene_power_shut_down_confirm,
    [SceneIdPowerUnplugUsb] = &system_settings_scene_power_unplug_usb,
    [SceneIdPowerRestartConfirm] = &system_settings_scene_power_restart_confirm,
    [SceneIdPowerRestart] = &system_settings_scene_power_restart,
    [SceneIdPowerInfo] = &system_settings_scene_power_info,

    [SceneIdDebug] = &system_settings_scene_debug,

    [SceneIdTelemetry] = &system_settings_scene_telemetry,

    [SceneIdFactoryResetConfirm] = &system_settings_scene_factory_reset_confirm,
    [SceneIdFactoryReset] = &system_settings_scene_factory_reset,
    [SceneIdLowBattery] = &system_settings_internal_scene_low_battery,
};
