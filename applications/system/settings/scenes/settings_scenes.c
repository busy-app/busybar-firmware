#include "settings_scenes.h"

extern const Scene settings_scene_start;
extern const Scene settings_scene_main;
extern const Scene settings_scene_sound;
extern const Scene settings_scene_brightness;
extern const Scene settings_scene_debug_apps;

extern const Scene settings_scene_matter;
extern const Scene settings_scene_matter_reset;
extern const Scene settings_scene_matter_pairing;
extern const Scene settings_scene_matter_commission_start;
extern const Scene settings_scene_matter_commission_fail;
extern const Scene settings_scene_matter_commission_done;

extern const Scene settings_scene_connect_wifi;

const Scene* const settings_scenes[SettingsAppSceneIdsCount] = {
    [SettingsAppSceneIdStart] = &settings_scene_start,
    [SettingsAppSceneIdMain] = &settings_scene_main,
    [SettingsAppSceneIdSound] = &settings_scene_sound,
    [SettingsAppSceneIdBrightness] = &settings_scene_brightness,
    [SettingsAppSceneIdDebugApps] = &settings_scene_debug_apps,

    [SettingsAppSceneIdMatter] = &settings_scene_matter,
    [SettingsAppSceneIdMatterReset] = &settings_scene_matter_reset,
    [SettingsAppSceneIdMatterPairing] = &settings_scene_matter_pairing,
    [SettingsAppSceneIdMatterCommissionStart] = &settings_scene_matter_commission_start,
    [SettingsAppSceneIdMatterCommissionFail] = &settings_scene_matter_commission_fail,
    [SettingsAppSceneIdMatterCommissionDone] = &settings_scene_matter_commission_done,

    [SettingsAppSceneIdConnectWifi] = &settings_scene_connect_wifi,
};
