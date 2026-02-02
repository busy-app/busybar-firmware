#include "brightness_scenes.h"

extern const Scene brightness_scene_main;

const Scene* const brightness_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &brightness_scene_main,
};
