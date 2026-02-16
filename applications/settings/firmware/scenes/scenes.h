#pragma once

#include <gui/scene_manager.h>

typedef enum {
    ThisSceneIdxMain,
    ThisSceneIdxSettings,
    ThisSceneIdxDialog,
    ThisSceneIdxCheck,
    ThisSceneIdxDownload,
    ThisSceneIdxResult,

    ThisSceneIdxsCount,
} ThisSceneIdx;

extern const Scene* const settings_firmware_app_scenes[];
