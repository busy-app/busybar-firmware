#include "apps_menu_scenes.h"

extern const Scene apps_menu_scene_start;
extern const Scene apps_menu_scene_main;
extern const Scene apps_menu_scene_js_app;
extern const Scene apps_menu_scene_coming_soon;

const Scene* const apps_menu_scenes[AppsMenuSceneIdMAX] = {
    [AppsMenuSceneIdStart] = &apps_menu_scene_start,
    [AppsMenuSceneIdMain] = &apps_menu_scene_main,
    [AppsMenuSceneIdJsApp] = &apps_menu_scene_js_app,
    [AppsMenuSceneIdComingSoon] = &apps_menu_scene_coming_soon,
};
