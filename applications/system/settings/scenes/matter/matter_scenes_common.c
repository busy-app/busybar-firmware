#include "matter_scenes_common.h"

bool matter_scene_replace_current(SettingsApp* app, SettingsCustomEvent event) {
    SettingsAppSceneId scene;

    if(event == SettingsCustomEventMatterCommStart) {
        scene = SettingsAppSceneIdMatterCommissionStart;
    } else if(event == SettingsCustomEventMatterCommComplete) {
        scene = SettingsAppSceneIdMatterCommissionDone;
    } else if(event == SettingsCustomEventMatterCommFail) {
        scene = SettingsAppSceneIdMatterCommissionFail;
    } else {
        return false;
    }

    scene_manager_replace_current_scene(app->scene_manager, scene);
    return true;
}
