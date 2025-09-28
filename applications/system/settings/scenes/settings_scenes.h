#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SettingsAppSceneIdStart,
    SettingsAppSceneIdMain,
    SettingsAppSceneIdSound,
    SettingsAppSceneIdBrightness,
    SettingsAppSceneIdDebugApps,

    SettingsAppSceneIdMatter,
    SettingsAppSceneIdMatterPairing,
    SettingsAppSceneIdMatterCommissionStart,
    SettingsAppSceneIdMatterCommissionFail,
    SettingsAppSceneIdMatterCommissionDone,

    SettingsAppSceneIdConnectWifi,
    SettingsAppSceneIdReboot,
    SettingsAppSceneIdAccount,

    SettingsAppSceneIdsCount
} SettingsAppSceneId;

extern const Scene* const settings_scenes[SettingsAppSceneIdsCount];
