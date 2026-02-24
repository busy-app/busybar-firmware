#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene clock_app_scene_main;
extern const Scene clock_app_scene_setup;
extern const Scene clock_app_scene_clock;

const Scene* const clock_app_scenes[] = {
    [ThisSceneIdxMain] = &clock_app_scene_main,
    [ThisSceneIdxClock] = &clock_app_scene_clock,
    [ThisSceneIdxSetup] = &clock_app_scene_setup,
};

static_assert(COUNT_OF(clock_app_scenes) == ThisSceneIdxsCount);
