#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdStarting,
    SceneIdAnimation,
    SceneIdUpdateFw,
    SceneIdMAX,
} SceneId;

extern const Scene* const power_on_scenes[SceneIdMAX];
