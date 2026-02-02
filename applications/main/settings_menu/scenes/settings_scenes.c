#include "settings_scenes.h"

extern const Scene settings_scene_start;
extern const Scene settings_scene_main;

const Scene* const settings_scenes[SettingsAppSceneIdsCount] = {
    [SettingsAppSceneIdStart] = &settings_scene_start,
    [SettingsAppSceneIdMain] = &settings_scene_main,
};
