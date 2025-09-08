#include "custom_scenes.h"

extern const Scene custom_scene_setup;
extern const Scene custom_scene_setup_theme;
extern const Scene custom_scene_start;
extern const Scene custom_scene_timer;

const Scene* const custom_scenes[CustomAppSceneIdMax] = {
    [CustomAppSceneIdStart] = &custom_scene_start,
    [CustomAppSceneIdTimer] = &custom_scene_timer,
    [CustomAppSceneIdSetup] = &custom_scene_setup,
    [CustomAppSceneIdSetupTheme] = &custom_scene_setup_theme,
};
