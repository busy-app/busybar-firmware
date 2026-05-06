#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,
    SceneIdPairing,

    SceneIdCommissionStart,
    SceneIdCommissionDone,
    SceneIdCommissionFail,

    SceneIdsCount,
} SceneId;

extern const Scene* const matter_scenes[SceneIdsCount];
