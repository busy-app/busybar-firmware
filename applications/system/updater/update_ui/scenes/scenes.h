#pragma once

#include <gui/scene_manager.h>

typedef enum {
    UpdateUiSceneIdxDownload,
    UpdateUiSceneIdxPrepare,
    UpdateUiSceneIdxFailure,

    UpdateUiSceneIdxsCount
} UpdateUiSceneIdx;

extern const Scene* const update_ui_internal_scenes[];
