#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene firmware_settings_internal_scene_main;
extern const Scene firmware_settings_internal_scene_settings;
extern const Scene firmware_settings_internal_scene_dialog;
extern const Scene firmware_settings_internal_scene_check;
extern const Scene firmware_settings_internal_scene_low_battery;
extern const Scene firmware_settings_internal_scene_check_result;

const Scene* const firmware_settings_internal_scenes[] = {
    [FirmwareSettingsSceneIdxMain] = &firmware_settings_internal_scene_main,
    [FirmwareSettingsSceneIdxSettings] = &firmware_settings_internal_scene_settings,
    [FirmwareSettingsSceneIdxCheck] = &firmware_settings_internal_scene_check,
    [FirmwareSettingsSceneIdxCheckResult] = &firmware_settings_internal_scene_check_result,
    [FirmwareSettingsSceneIdxDialog] = &firmware_settings_internal_scene_dialog,
    [FirmwareSettingsSceneIdxLowBattery] = &firmware_settings_internal_scene_low_battery,
};

static_assert(COUNT_OF(firmware_settings_internal_scenes) == FirmwareSettingsSceneIdxsCount);
