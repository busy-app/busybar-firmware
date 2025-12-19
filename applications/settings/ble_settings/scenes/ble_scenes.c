#include "ble_scenes.h"

extern const Scene ble_scene_pairing_mode;
extern const Scene ble_scene_forget_device;
extern const Scene ble_scene_forget_device_confirm;

const Scene* const ble_settings_scenes[SceneIdsCount] = {
    [SceneIdPairingMode] = &ble_scene_pairing_mode,
    [SceneIdForgetDevice] = &ble_scene_forget_device,
    [SceneIdForgetDeviceConfirm] = &ble_scene_forget_device_confirm,

};
