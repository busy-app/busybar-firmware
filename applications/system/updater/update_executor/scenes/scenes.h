#pragma once

#include <gui/scene_manager.h>

typedef enum {
    UpdateExecutorSceneIdxInstall,
    UpdateExecutorSceneIdxFailure,
    UpdateExecutorSceneIdxSuccess,
    UpdateExecutorSceneIdxReboot,

    UpdateExecutorSceneIdxsCount
} UpdateExecutorSceneIdx;

extern const Scene* const update_executor_internal_scenes[];
