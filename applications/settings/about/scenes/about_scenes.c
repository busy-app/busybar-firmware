#include "about_scenes.h"

extern const Scene about_scene_main;

extern const Scene about_scene_general;
extern const Scene about_scene_firmware;
extern const Scene about_scene_compliance;
extern const Scene about_scene_libs_list;
extern const Scene about_scene_lib_info;

const Scene* const about_scenes[SceneIdsCount] = {
    [SceneIdMain] = &about_scene_main,

    [SceneIdGeneral] = &about_scene_general,
    [SceneIdFirmware] = &about_scene_firmware,
    [SceneIdCompliance] = &about_scene_compliance,

    [SceneIdLibsList] = &about_scene_libs_list,
    [SceneIdLibInfo] = &about_scene_lib_info,
};
