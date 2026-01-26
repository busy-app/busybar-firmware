#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,

    SceneIdFactoryResetConfirm,
    SceneIdFactoryReset,

    SceneIdPowerMenu,
    SceneIdPowerShutDownConfirm,
    SceneIdPowerUnplugUsb,
    SceneIdPowerRestart,

    SceneIdsCount,
} SceneId;

extern const Scene* const system_settings_scenes[SceneIdsCount];
