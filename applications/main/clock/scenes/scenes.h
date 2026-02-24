#pragma once

#include <gui/scene_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ThisSceneIdxMain,
    ThisSceneIdxClock,
    ThisSceneIdxSetup,

    ThisSceneIdxsCount,
} ThisSceneIdx;

extern const Scene* const clock_app_scenes[];

#ifdef __cplusplus
}
#endif
