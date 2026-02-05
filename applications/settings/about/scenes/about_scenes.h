#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,

    SceneIdGeneral,
    SceneIdFirmware,
    SceneIdCompliance,

    SceneIdsCount,
} SceneId;

extern const Scene* const about_scenes[SceneIdsCount];
