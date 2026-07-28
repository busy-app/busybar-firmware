#include "../js_app_launcher_i.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_menu.h>

typedef struct {
    AnimMenu* front_menu;
    Menu* back_menu;
} JsAppLauncherSceneStart;

static void js_app_launcher_scene_start_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    // TODO: Implementation
    FURI_LOG_I(TAG, "Running JS application with id \"%s\"", instance->app_id);
}

static void js_app_launcher_scene_start_on_exit(void* context) {
    furi_assert(context);
}

static bool js_app_launcher_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
    }

    return consumed;
}

const Scene js_app_launcher_scene_start = {
    .data_size = sizeof(JsAppLauncherSceneStart),
    .enter_callback = js_app_launcher_scene_start_on_enter,
    .exit_callback = js_app_launcher_scene_start_on_exit,
    .event_callback = js_app_launcher_scene_start_on_event,
};
