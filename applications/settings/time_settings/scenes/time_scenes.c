#include "time_scenes.h"

extern const Scene time_scene_menu;
extern const Scene time_scene_timezone;
extern const Scene time_scene_format;

const Scene* const time_settings_scenes[SceneIdsCount] = {
    [SceneIdMenu] = &time_scene_menu,
    [SceneIdTimezone] = &time_scene_timezone,
    [SceneIdFormat] = &time_scene_format,
};
