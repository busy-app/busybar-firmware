#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene update_ui_internal_scene_download;
extern const Scene update_ui_internal_scene_prepare;
extern const Scene update_ui_internal_scene_failure;

const Scene* const update_ui_internal_scenes[] = {
    [UpdateUiSceneIdxDownload] = &update_ui_internal_scene_download,
    [UpdateUiSceneIdxPrepare] = &update_ui_internal_scene_prepare,
    [UpdateUiSceneIdxFailure] = &update_ui_internal_scene_failure,
};

static_assert(COUNT_OF(update_ui_internal_scenes) == UpdateUiSceneIdxsCount);
