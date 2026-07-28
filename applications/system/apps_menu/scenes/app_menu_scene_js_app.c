#include "../apps_menu_i.h"
#include "../storage_macros.h"
#include "apps_menu_scenes.h"

typedef struct {
    uint8_t dummy;
} AppsMenuSceneJsApp;

static void apps_menu_scene_js_app_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;

    // TODO: Implementation
    FURI_LOG_I(
        "AppsMenu",
        "Running JS application with id \"%s\"",
        instance->settings.active_application);
}

static void apps_menu_scene_js_app_on_exit(void* context) {
    furi_assert(context);
}

static bool apps_menu_scene_js_app_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
    }

    return consumed;
}

const Scene apps_menu_scene_js_app = {
    .enter_callback = apps_menu_scene_js_app_on_enter,
    .exit_callback = apps_menu_scene_js_app_on_exit,
    .event_callback = apps_menu_scene_js_app_on_event,
    .data_size = sizeof(AppsMenuSceneJsApp),
};
