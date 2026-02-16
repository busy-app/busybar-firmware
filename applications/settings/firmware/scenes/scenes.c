#include "scenes.h"

#include <furi/core/core_defines.h>

#include <assert.h>

extern const Scene settings_firmware_app_scene_main;
extern const Scene settings_firmware_app_scene_settings;
extern const Scene settings_firmware_app_scene_dialog;
extern const Scene settings_firmware_app_scene_check;
extern const Scene settings_firmware_app_scene_download;
extern const Scene settings_firmware_app_scene_result;

const Scene* const settings_firmware_app_scenes[] = {
    [ThisSceneIdxMain] = &settings_firmware_app_scene_main,
    [ThisSceneIdxSettings] = &settings_firmware_app_scene_settings,
    [ThisSceneIdxDialog] = &settings_firmware_app_scene_dialog,
    [ThisSceneIdxCheck] = &settings_firmware_app_scene_check,
    [ThisSceneIdxDownload] = &settings_firmware_app_scene_download,
    [ThisSceneIdxResult] = &settings_firmware_app_scene_result,
};

static_assert(COUNT_OF(settings_firmware_app_scenes) == ThisSceneIdxsCount);
