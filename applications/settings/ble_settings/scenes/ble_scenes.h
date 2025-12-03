#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdPairingMode,
    SceneIdForgetDevice,
    SceneIdForgetDeviceConfirm,

    SceneIdMain, //TODO: remove this and main scene also
    SceneIdsCount = 2,
} SceneId;

extern const Scene* const ble_settings_scenes[SceneIdsCount];
