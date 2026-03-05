#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdConnecting,
    SceneIdNotLinkedMenu,
    SceneIdLinkPin,
    SceneIdLinkedInfo,
    SceneIdLinkedMenu,
    SceneIdUnlink,
    SceneIdError,
    SceneIdsCount,
} SceneId;

extern const Scene* const account_settings_scenes[SceneIdsCount];
