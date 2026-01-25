#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,
    SceneIdFactoryResetConfirm,
    SceneIdFactoryReset,

    SceneIdsCount,
} SceneId;

extern const Scene* const system_settings_scenes[SceneIdsCount];
