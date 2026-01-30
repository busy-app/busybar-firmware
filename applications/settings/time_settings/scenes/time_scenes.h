#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMenu,
    SceneIdTimezone,
    // SceneIdFormat,
    SceneIdsCount,
} SceneId;

extern const Scene* const time_settings_scenes[SceneIdsCount];
