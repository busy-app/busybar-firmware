#include "apps_menu_scenes.h"

extern const Scene apps_menu_scene_start;
extern const Scene apps_menu_scene_main;

const Scene* const apps_menu_scenes[AppsMenuSceneIdMAX] = {
    [AppsMenuSceneIdStart] = &apps_menu_scene_start,
    [AppsMenuSceneIdMain] = &apps_menu_scene_main,
};
