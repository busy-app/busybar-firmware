#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SettingsAppSceneIdStart,
    SettingsAppSceneIdMain,
    SettingsAppSceneIdSound,
    SettingsAppSceneIdBrightness,
    SettingsAppSceneIdDebugApps,
    SettingsAppSceneIdMatter,
    SettingsAppSceneIdMatterReset,
    SettingsAppSceneIdMatterPairing,
    SettingsAppSceneIdMatterCommissionStart,
    SettingsAppSceneIdMatterCommissionFail,
    SettingsAppSceneIdMatterCommissionDone,

    SettingsAppSceneIdsCount
} SettingsAppSceneId;

extern const Scene* const settings_scenes[SettingsAppSceneIdsCount];
