#pragma once

#include <gui/scene_manager.h>

typedef enum {
    ThisSceneIdxMain,
    ThisSceneIdxSettings,
    ThisSceneIdxCheck,
    ThisSceneIdxCheckResult,
    ThisSceneIdxDialog,
    ThisSceneIdxLowBattery,

    ThisSceneIdxsCount,
} ThisSceneIdx;

extern const Scene* const settings_firmware_internal_scenes[];
