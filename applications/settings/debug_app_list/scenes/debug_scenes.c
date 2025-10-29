#include "debug_scenes.h"

extern const Scene debug_scene_main;

const Scene* const debug_scenes[SceneIdsCount] = {
    [SceneIdMain] = &debug_scene_main,
};
