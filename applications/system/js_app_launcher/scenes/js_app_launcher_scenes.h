#pragma once

#include <gui/scene_manager.h>

typedef enum {
    JsAppLauncherSceneIdStart,
    JsAppLauncherSceneIdRun,
    JsAppLauncherSceneIdSetup,
    JsAppLauncherSceneIdError,
    JsAppLauncherSceneIdWebSetup,
    JsAppLauncherSceneIdMax,
} JsAppLauncherSceneId;

extern const Scene* const js_app_launcher_scenes[JsAppLauncherSceneIdMax];
