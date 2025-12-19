#pragma once

#include <gui/scene_manager.h>

typedef enum {
    SceneIdPairingMode,
    SceneIdForgetDevice,
    SceneIdForgetDeviceConfirm,

    SceneIdsCount
} SceneId;

extern const Scene* const ble_settings_scenes[SceneIdsCount];
