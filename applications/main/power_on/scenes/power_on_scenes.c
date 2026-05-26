#include "power_on_scenes.h"

extern const Scene power_on_scene_starting;
extern const Scene power_on_scene_animation;
extern const Scene power_on_scene_update_fw;

const Scene* const power_on_scenes[SceneIdMAX] = {
    [SceneIdStarting] = &power_on_scene_starting,
    [SceneIdAnimation] = &power_on_scene_animation,
    [SceneIdUpdateFw] = &power_on_scene_update_fw,
};
