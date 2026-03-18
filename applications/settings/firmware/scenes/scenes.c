#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene settings_firmware_internal_scene_main;
extern const Scene settings_firmware_internal_scene_settings;
extern const Scene settings_firmware_internal_scene_dialog;
extern const Scene settings_firmware_internal_scene_check;
extern const Scene settings_firmware_internal_scene_low_battery;
extern const Scene settings_firmware_internal_scene_check_result;

const Scene* const settings_firmware_internal_scenes[] = {
    [ThisSceneIdxMain] = &settings_firmware_internal_scene_main,
    [ThisSceneIdxSettings] = &settings_firmware_internal_scene_settings,
    [ThisSceneIdxCheck] = &settings_firmware_internal_scene_check,
    [ThisSceneIdxCheckResult] = &settings_firmware_internal_scene_check_result,
    [ThisSceneIdxDialog] = &settings_firmware_internal_scene_dialog,
    [ThisSceneIdxLowBattery] = &settings_firmware_internal_scene_low_battery,
};

static_assert(COUNT_OF(settings_firmware_internal_scenes) == ThisSceneIdxsCount);
