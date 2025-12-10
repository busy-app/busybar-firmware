#pragma once

#include <gui/scene_manager.h>

typedef enum {
    UpdateExecutorSceneIdInstall,
    UpdateExecutorSceneIdSuccess,
    UpdateExecutorSceneIdFail,

    UpdateExecutorSceneIdsCount
} UpdateExecutorSceneId;

extern const Scene* const update_executor_scenes[];
