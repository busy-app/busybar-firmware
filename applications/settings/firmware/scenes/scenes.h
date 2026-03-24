#pragma once

#include <gui/scene_manager.h>

typedef enum {
    FirmwareSettingsSceneIdxMain,
    FirmwareSettingsSceneIdxSettings,
    FirmwareSettingsSceneIdxCheck,
    FirmwareSettingsSceneIdxCheckResult,
    FirmwareSettingsSceneIdxDialog,
    FirmwareSettingsSceneIdxLowBattery,

    FirmwareSettingsSceneIdxsCount,
} FirmwareSettingsSceneIdx;

extern const Scene* const firmware_settings_internal_scenes[];
