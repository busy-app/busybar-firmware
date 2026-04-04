#pragma once

#include <gui/scene_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ClockSceneIdxMain,
    ClockSceneIdxClock,
    ClockSceneIdxSetup,

    ClockSceneIdxsCount,
} ClockSceneIdx;

extern const Scene* const clock_internal_scenes[];

#ifdef __cplusplus
}
#endif
