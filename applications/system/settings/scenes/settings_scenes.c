#include "settings_scenes.h"

extern const Scene settings_scene_start;
extern const Scene settings_scene_main;
extern const Scene settings_scene_sound;
extern const Scene settings_scene_brightness;
extern const Scene settings_scene_debug_apps;
extern const Scene settings_scene_fw_update;

extern const Scene settings_scene_matter;
extern const Scene settings_scene_matter_pairing;
extern const Scene settings_scene_matter_commission_start;
extern const Scene settings_scene_matter_commission_fail;
extern const Scene settings_scene_matter_commission_done;

extern const Scene settings_scene_connect_wifi;
extern const Scene settings_scene_reboot;
extern const Scene settings_scene_account;

const Scene* const settings_scenes[SettingsAppSceneIdsCount] = {
    [SettingsAppSceneIdStart] = &settings_scene_start,
    [SettingsAppSceneIdMain] = &settings_scene_main,
    [SettingsAppSceneIdSound] = &settings_scene_sound,
    [SettingsAppSceneIdBrightness] = &settings_scene_brightness,
    [SettingsAppSceneIdDebugApps] = &settings_scene_debug_apps,

    [SettingsAppSceneIdFwUpdate] = &settings_scene_fw_update,

    [SettingsAppSceneIdMatter] = &settings_scene_matter,
    [SettingsAppSceneIdMatterPairing] = &settings_scene_matter_pairing,
    [SettingsAppSceneIdMatterCommissionStart] = &settings_scene_matter_commission_start,
    [SettingsAppSceneIdMatterCommissionFail] = &settings_scene_matter_commission_fail,
    [SettingsAppSceneIdMatterCommissionDone] = &settings_scene_matter_commission_done,

    [SettingsAppSceneIdConnectWifi] = &settings_scene_connect_wifi,
    [SettingsAppSceneIdReboot] = &settings_scene_reboot,
    [SettingsAppSceneIdAccount] = &settings_scene_account,
};
