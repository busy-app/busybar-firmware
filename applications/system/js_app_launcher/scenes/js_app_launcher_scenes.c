#include "js_app_launcher_scenes.h"

extern const Scene js_app_launcher_scene_start;
extern const Scene js_app_launcher_scene_run;
extern const Scene js_app_launcher_scene_setup;
extern const Scene js_app_launcher_scene_error;
extern const Scene js_app_launcher_scene_web_setup;

const Scene* const js_app_launcher_scenes[] = {
    [JsAppLauncherSceneIdStart] = &js_app_launcher_scene_start,
    [JsAppLauncherSceneIdRun] = &js_app_launcher_scene_run,
    [JsAppLauncherSceneIdSetup] = &js_app_launcher_scene_setup,
    [JsAppLauncherSceneIdError] = &js_app_launcher_scene_error,
    [JsAppLauncherSceneIdWebSetup] = &js_app_launcher_scene_web_setup,
};
