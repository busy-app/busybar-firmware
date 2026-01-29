#include "update_scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene settings_firmware_app_scene_main;

const Scene* const settings_firmware_app_scenes[] = {
    [ThisSceneIdxMain] = &settings_firmware_app_scene_main,
};

static_assert(COUNT_OF(settings_firmware_app_scenes) == ThisSceneIdxsCount);
