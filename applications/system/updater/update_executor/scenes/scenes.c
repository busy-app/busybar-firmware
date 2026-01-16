#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene update_executor_scene_install;
extern const Scene update_executor_scene_success;
extern const Scene update_executor_scene_fail;

const Scene* const update_executor_scenes[] = {
    [UpdateExecutorSceneIdInstall] = &update_executor_scene_install,
    [UpdateExecutorSceneIdSuccess] = &update_executor_scene_success,
    [UpdateExecutorSceneIdFail] = &update_executor_scene_fail,
};

static_assert(COUNT_OF(update_executor_scenes) == UpdateExecutorSceneIdsCount);
