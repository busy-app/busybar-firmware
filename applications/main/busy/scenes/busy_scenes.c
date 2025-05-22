#include "busy_scenes.h"

extern const Scene busy_scene_next;
extern const Scene busy_scene_overview;
extern const Scene busy_scene_quit;
extern const Scene busy_scene_progress;
extern const Scene busy_scene_setup;
extern const Scene busy_scene_setup_timer;
extern const Scene busy_scene_setup_theme;
extern const Scene busy_scene_start;
extern const Scene busy_scene_static;
extern const Scene busy_scene_timer;

const Scene* const busy_scenes[BusyAppSceneIdMax] = {
    [BusyAppSceneIdStart] = &busy_scene_start,
    [BusyAppSceneIdOverview] = &busy_scene_overview,
    [BusyAppSceneIdTimer] = &busy_scene_timer,
    [BusyAppSceneIdStatic] = &busy_scene_static,
    [BusyAppSceneIdNext] = &busy_scene_next,
    [BusyAppSceneIdQuit] = &busy_scene_quit,
    [BusyAppSceneIdProgress] = &busy_scene_progress,
    [BusyAppSceneIdSetup] = &busy_scene_setup,
    [BusyAppSceneIdSetupTimer] = &busy_scene_setup_timer,
    [BusyAppSceneIdSetupTheme] = &busy_scene_setup_theme,
};
