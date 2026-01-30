#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene settings_firmware_app_scene_main;
extern const Scene settings_firmware_app_scene_settings;

const Scene* const settings_firmware_app_scenes[] = {
    [ThisSceneIdxMain] = &settings_firmware_app_scene_main,
    [ThisSceneIdxSettings] = &settings_firmware_app_scene_settings,
};

static_assert(COUNT_OF(settings_firmware_app_scenes) == ThisSceneIdxsCount);
