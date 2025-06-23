#pragma once

#include "../scene_manager.h"

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdOverview,
    BusyAppSceneIdTimer,
    BusyAppSceneIdNext,
    BusyAppSceneIdProgress,
    BusyAppSceneIdSetup,
    BusyAppSceneIdSetupTimer,
    BusyAppSceneIdSetupTheme,
    BusyAppSceneIdMax,
} BusyAppSceneId;

extern const Scene* const busy_scenes[BusyAppSceneIdMax];
