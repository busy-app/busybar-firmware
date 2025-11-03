#include "updater_scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene updater_scene_install;
extern const Scene updater_scene_success;
extern const Scene updater_scene_fail;

const Scene* const updater_scenes[] = {
    [UpdaterAppSceneIdInstall] = &updater_scene_install,
    [UpdaterAppSceneIdSuccess] = &updater_scene_success,
    [UpdaterAppSceneIdFail] = &updater_scene_fail,
};

static_assert(COUNT_OF(updater_scenes) == UpdaterAppSceneIdsCount);
