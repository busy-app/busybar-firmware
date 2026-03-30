#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene clock_internal_scene_main;
extern const Scene clock_internal_scene_setup;
extern const Scene clock_internal_scene_clock;

const Scene* const clock_internal_scenes[] = {
    [ClockSceneIdxMain] = &clock_internal_scene_main,
    [ClockSceneIdxClock] = &clock_internal_scene_clock,
    [ClockSceneIdxSetup] = &clock_internal_scene_setup,
};

static_assert(COUNT_OF(clock_internal_scenes) == ClockSceneIdxsCount);
