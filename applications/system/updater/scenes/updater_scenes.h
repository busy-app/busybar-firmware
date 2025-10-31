#pragma once

#include <gui/scene_manager.h>

typedef enum {
    UpdaterAppSceneIdInstall,
    UpdaterAppSceneIdSuccess,
    UpdaterAppSceneIdFail,

    UpdaterAppSceneIdsCount
} UpdaterAppSceneId;

extern const Scene* const updater_scenes[];
