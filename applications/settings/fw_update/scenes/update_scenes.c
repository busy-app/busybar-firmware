#include "update_scenes.h"

extern const Scene fw_update_scene_main;

const Scene* const fw_update_scenes[SceneIdsCount] = {
    [SceneIdMain] = &fw_update_scene_main,
};
