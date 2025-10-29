#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,

    SceneIdsCount,
} SceneId;

extern const Scene* const debug_scenes[SceneIdsCount];
