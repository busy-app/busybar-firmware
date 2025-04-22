#pragma once

#include "../scene_manager.h"

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdTimer,
    BusyAppSceneIdStatic,
    BusyAppSceneIdQuit,
    BusyAppSceneIdNext,
    BusyAppSceneIdRestart,
    BusyAppSceneIdSetup,
    BusyAppSceneIdSetupTimer,
    BusyAppSceneIdMax,
} BusyAppSceneId;

extern const Scene* const busy_scenes[BusyAppSceneIdMax];
