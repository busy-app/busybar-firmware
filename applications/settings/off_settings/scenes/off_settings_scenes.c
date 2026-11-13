#include "off_settings_scenes.h"

extern const Scene off_settings_scene_main;

const Scene* const off_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &off_settings_scene_main,
};
