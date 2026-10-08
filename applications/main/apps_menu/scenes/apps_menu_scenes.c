#include "apps_menu_scenes.h"

#include <core/check.h>

extern const Scene apps_menu_scene_start;
extern const Scene apps_menu_scene_main;

const Scene* const apps_menu_scenes[] = {
    [AppsMenuSceneIdStart] = &apps_menu_scene_start,
    [AppsMenuSceneIdMain] = &apps_menu_scene_main,
};

static_assert(COUNT_OF(apps_menu_scenes) == AppsMenuSceneIdMax);
