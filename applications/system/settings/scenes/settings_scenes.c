#include "settings_scenes.h"

extern const Scene settings_scene_start;
extern const Scene settings_scene_main;
extern const Scene settings_scene_sound;
extern const Scene settings_scene_brightness;
extern const Scene settings_scene_debug_apps;
extern const Scene settings_scene_fw_update;

const Scene* const settings_scenes[SettingsAppSceneIdsCount] = {
    [SettingsAppSceneIdStart] = &settings_scene_start,
    [SettingsAppSceneIdMain] = &settings_scene_main,
    [SettingsAppSceneIdSound] = &settings_scene_sound,
    [SettingsAppSceneIdBrightness] = &settings_scene_brightness,
    [SettingsAppSceneIdDebugApps] = &settings_scene_debug_apps,
    [SettingsAppSceneIdFwUpdate] = &settings_scene_fw_update,
};
