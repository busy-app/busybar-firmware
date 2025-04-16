#include "busy_scenes.h"

extern const Scene busy_scene_next;
extern const Scene busy_scene_quit;
extern const Scene busy_scene_restart;
extern const Scene busy_scene_setup;
extern const Scene busy_scene_start;
extern const Scene busy_scene_static;
extern const Scene busy_scene_timer;

const Scene* const busy_scenes[BusyAppSceneIdMax] = {
    [BusyAppSceneIdStart] = &busy_scene_start,
    [BusyAppSceneIdTimer] = &busy_scene_timer,
    [BusyAppSceneIdStatic] = &busy_scene_static,
    [BusyAppSceneIdQuit] = &busy_scene_quit,
    [BusyAppSceneIdNext] = &busy_scene_next,
    [BusyAppSceneIdRestart] = &busy_scene_restart,
    [BusyAppSceneIdSetup] = &busy_scene_setup,
};
