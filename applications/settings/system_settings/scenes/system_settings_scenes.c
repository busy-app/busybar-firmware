#include "system_settings_scenes.h"

extern const Scene system_settings_scene_main;
extern const Scene system_settings_scene_factory_reset_confirm;
extern const Scene system_settings_scene_factory_reset;

const Scene* const system_settings_scenes[SceneIdsCount] = {
    [SceneIdMain] = &system_settings_scene_main,
    [SceneIdFactoryResetConfirm] = &system_settings_scene_factory_reset_confirm,
    [SceneIdFactoryReset] = &system_settings_scene_factory_reset,
};
