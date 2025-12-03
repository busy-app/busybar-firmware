#include "ble_scenes.h"

extern const Scene ble_scene_main;
extern const Scene ble_scene_forget_device;

const Scene* const ble_settings_scenes[SceneIdsCount] = {
    [SceneIdPairingMode] = &ble_scene_main,
    [SceneIdForgetDevice] = &ble_scene_forget_device,
    // [SceneIdForgetDevice]
    //[SceneIdForgetDeviceConfirm]

};
