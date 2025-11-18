#include "ble_scenes.h"

extern const Scene ble_scene_main;

const Scene* const ble_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &ble_scene_main,
};
