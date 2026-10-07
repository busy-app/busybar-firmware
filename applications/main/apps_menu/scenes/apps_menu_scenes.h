#pragma once

#include <gui/scene_manager.h>

typedef enum {
    AppsMenuSceneIdStart,
    AppsMenuSceneIdMain,

    AppsMenuSceneIdMAX
} AppsMenuSceneId;

extern const Scene* const apps_menu_scenes[AppsMenuSceneIdMAX];
