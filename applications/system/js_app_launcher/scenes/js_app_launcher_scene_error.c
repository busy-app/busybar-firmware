#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

typedef struct {
    bool dummy;
} JsAppLauncherSceneError;

static void js_app_launcher_scene_error_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    UNUSED(instance);
}

static void js_app_launcher_scene_error_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    UNUSED(instance);
}

static bool js_app_launcher_scene_error_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    JsAppLauncher* instance = context;
    UNUSED(instance);

    return consumed;
}

const Scene js_app_launcher_scene_error = {
    .data_size = sizeof(JsAppLauncherSceneError),
    .enter_callback = js_app_launcher_scene_error_on_enter,
    .exit_callback = js_app_launcher_scene_error_on_exit,
    .event_callback = js_app_launcher_scene_error_on_event,
};
