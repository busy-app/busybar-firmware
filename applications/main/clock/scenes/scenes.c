#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene clock_app_scene_main;

const Scene* const clock_app_scenes[] = {
    [ThisSceneIdxMain] = &clock_app_scene_main,
};

static_assert(COUNT_OF(clock_app_scenes) == ThisSceneIdxsCount);
