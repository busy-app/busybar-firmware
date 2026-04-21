#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdNotConnected,
    SceneIdState,
    SceneIdMenu,
    SceneIdInfo,
    SceneIdForget,
    SceneIdDisconnecting,
    SceneIdsCount,
} SceneId;

extern const Scene* const wifi_settings_scenes[SceneIdsCount];
