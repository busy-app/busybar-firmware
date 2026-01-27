#include "system_settings_scenes.h"

extern const Scene system_settings_scene_main;
extern const Scene system_settings_scene_factory_reset_confirm;
extern const Scene system_settings_scene_factory_reset;
extern const Scene system_settings_scene_power_menu;
extern const Scene system_settings_scene_power_shut_down_confirm;
extern const Scene system_settings_scene_power_unplug_usb;
extern const Scene system_settings_scene_power_restart;
extern const Scene system_settings_scene_power_info;
extern const Scene system_settings_scene_debug;

const Scene* const system_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &system_settings_scene_main,

    [SceneIdFactoryResetConfirm] = &system_settings_scene_factory_reset_confirm,
    [SceneIdFactoryReset] = &system_settings_scene_factory_reset,

    [SceneIdPowerMenu] = &system_settings_scene_power_menu,
    [SceneIdPowerShutDownConfirm] = &system_settings_scene_power_shut_down_confirm,
    [SceneIdPowerUnplugUsb] = &system_settings_scene_power_unplug_usb,
    [SceneIdPowerRestart] = &system_settings_scene_power_restart,
    [SceneIdPowerInfo] = &system_settings_scene_power_info,

    [SceneIdDebug] = &system_settings_scene_debug,
};
