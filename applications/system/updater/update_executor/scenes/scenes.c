#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene update_executor_internal_scene_install;
extern const Scene update_executor_internal_scene_failure;
extern const Scene update_executor_internal_scene_success;
extern const Scene update_executor_internal_scene_reboot;

const Scene* const update_executor_internal_scenes[] = {
    [UpdateExecutorSceneIdxInstall] = &update_executor_internal_scene_install,
    [UpdateExecutorSceneIdxFailure] = &update_executor_internal_scene_failure,
    [UpdateExecutorSceneIdxSuccess] = &update_executor_internal_scene_success,
    [UpdateExecutorSceneIdxReboot] = &update_executor_internal_scene_reboot,
};

static_assert(COUNT_OF(update_executor_internal_scenes) == UpdateExecutorSceneIdxsCount);
