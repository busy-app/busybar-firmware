#include "custom_scenes.h"

// extern const Scene custom_scene_next;
// extern const Scene custom_scene_overview;
// extern const Scene custom_scene_progress;
extern const Scene custom_scene_setup;
// extern const Scene custom_scene_setup_timer;
extern const Scene custom_scene_setup_theme;
extern const Scene custom_scene_start;
extern const Scene custom_scene_timer;

const Scene* const custom_scenes[CustomAppSceneIdMax] = {
    [CustomAppSceneIdStart] = &custom_scene_start,
    // [CustomAppSceneIdOverview] = &custom_scene_overview,
    [CustomAppSceneIdTimer] = &custom_scene_timer,
    // [CustomAppSceneIdNext] = &custom_scene_next,
    // [CustomAppSceneIdProgress] = &custom_scene_progress,
    [CustomAppSceneIdSetup] = &custom_scene_setup,
    // [CustomAppSceneIdSetupTimer] = &custom_scene_setup_timer,
    [CustomAppSceneIdSetupTheme] = &custom_scene_setup_theme,
};
