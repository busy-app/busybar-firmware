#pragma once

#include "../scene_manager.h"

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdTimer,
    BusyAppSceneIdStatic,
    BusyAppSceneIdNext,
    BusyAppSceneIdQuit,
    BusyAppSceneIdProgress,
    BusyAppSceneIdSetup,
    BusyAppSceneIdSetupTimer,
    BusyAppSceneIdSetupTheme,
    BusyAppSceneIdMax,
} BusyAppSceneId;

extern const Scene* const busy_scenes[BusyAppSceneIdMax];
