#include "js_app_launcher_scenes.h"

extern const Scene js_app_launcher_scene_start;

const Scene* const js_app_launcher_scenes[] = {
    [JsAppLauncherSceneIdStart] = &js_app_launcher_scene_start,
};
