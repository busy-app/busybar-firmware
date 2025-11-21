#include "busy_scenes.h"

extern const Scene busy_scene_next;
extern const Scene busy_scene_overview;
extern const Scene busy_scene_progress;
extern const Scene busy_scene_setup;
extern const Scene busy_scene_setup_timer;
extern const Scene busy_scene_setup_theme;
extern const Scene busy_scene_start;
extern const Scene busy_scene_timer_interval;
extern const Scene busy_scene_timer_off;
extern const Scene busy_scene_timer_off_to_simple;
extern const Scene busy_scene_timer_simple;
extern const Scene busy_scene_timer_old;

const Scene* const busy_scenes[BusyAppSceneIdMax] = {
    [BusyAppSceneIdStart] = &busy_scene_start,
    [BusyAppSceneIdOverview] = &busy_scene_overview,
    [BusyAppSceneIdTimerOff] = &busy_scene_timer_off,
    [BusyAppSceneIdTimerOffToSimple] = &busy_scene_timer_off_to_simple,
    [BusyAppSceneIdTimerSimple] = &busy_scene_timer_simple,
    [BusyAppSceneIdTimerOld] = &busy_scene_timer_old,
    [BusyAppSceneIdNext] = &busy_scene_next,
    [BusyAppSceneIdProgress] = &busy_scene_progress,
    [BusyAppSceneIdSetup] = &busy_scene_setup,
    [BusyAppSceneIdSetupTimer] = &busy_scene_setup_timer,
    [BusyAppSceneIdSetupTheme] = &busy_scene_setup_theme,
    [BusyAppSceneIdTimerInterval] = &busy_scene_timer_interval,
};
