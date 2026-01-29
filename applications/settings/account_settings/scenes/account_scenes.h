#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdConnecting,
    SceneIdNotLinked,
    SceneIdLinkPin,
    SceneIdLinked,
    SceneIdError,
    SceneIdNoWifi,
    SceneIdsCount,
} SceneId;

extern const Scene* const account_settings_scenes[SceneIdsCount];
