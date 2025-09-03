#pragma once

#include <gui/scene_manager.h>

typedef enum {
    CustomAppSceneIdStart,
    CustomAppSceneIdTimer,
    CustomAppSceneIdSetup,
    CustomAppSceneIdSetupTheme,
    CustomAppSceneIdMax,
} CustomAppSceneId;

extern const Scene* const custom_scenes[CustomAppSceneIdMax];
