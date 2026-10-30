#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,
    SceneIdsCount,
} SceneId;

extern const Scene* const off_settings_scenes[SceneIdsCount];
