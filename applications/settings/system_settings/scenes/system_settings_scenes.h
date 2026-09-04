#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,

    SceneIdPowerMenu,
    SceneIdPowerShutDownConfirm,
    SceneIdPowerUnplugUsb,
    SceneIdPowerRestartConfirm,
    SceneIdPowerRestart,
    SceneIdPowerInfo,

    SceneIdDebug,

    SceneIdTelemetry,

    SceneIdFactoryResetConfirm,
    SceneIdFactoryReset,
    SceneIdLowBattery,

    SceneIdsCount,
} SceneId;

extern const Scene* const system_settings_scenes[SceneIdsCount];
