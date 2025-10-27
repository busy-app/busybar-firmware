#pragma once

#include <gui/scene_manager.h>

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdOverview,
    BusyAppSceneIdTimerOff,
    BusyAppSceneIdTimerOffToSimple,
    BusyAppSceneIdTimerSimple,
    BusyAppSceneIdTimerOld,
    BusyAppSceneIdNext,
    BusyAppSceneIdProgress,
    BusyAppSceneIdSetup,
    BusyAppSceneIdSetupTimer,
    BusyAppSceneIdSetupTheme,
    BusyAppSceneIdMax,
} BusyAppSceneId;

extern const Scene* const busy_scenes[BusyAppSceneIdMax];
