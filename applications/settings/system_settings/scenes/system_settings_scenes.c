#include "system_settings_scenes.h"

extern const Scene system_scene_main;

const Scene* const system_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &system_scene_main,
};
