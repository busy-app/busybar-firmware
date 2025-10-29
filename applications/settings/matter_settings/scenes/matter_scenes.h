#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdMain,
    SceneIdPairing,

    SceneIdCommissionStart,
    SceneIdCommissionDone,
    SceneIdCommissionFail,

    SceneIdConnectWifi,
    SceneIdReboot,

    SceneIdsCount,
} SceneId;

extern const Scene* const matter_scenes[SceneIdsCount];
