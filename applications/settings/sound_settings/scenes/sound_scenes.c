#include "sound_scenes.h"

extern const Scene sound_scene_main;

const Scene* const sound_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &sound_scene_main,
};
