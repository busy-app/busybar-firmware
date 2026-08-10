#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/label.h>

typedef struct {
    Label* front_label;
    Label* back_label;
} JsAppLauncherSceneSetup;

static void js_app_launcher_scene_setup_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    with_gui(instance->gui, {
        nav_bar_push_location(instance->nav_bar, "SETUP");

        data->front_label = label_alloc(instance->front_window);
        label_set_text(data->front_label, "Not implemented");
        widget_set_align(label_get_base(data->front_label), AlignCenter);

        data->back_label = label_alloc(instance->back_window);
        label_set_text(data->back_label, "Not implemented");
        widget_set_align(label_get_base(data->back_label), AlignCenter);
    });
}

static void js_app_launcher_scene_setup_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->nav_bar);

        label_free(data->front_label);
        label_free(data->back_label);
    });
}

static bool js_app_launcher_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    JsAppLauncher* instance = context;
    UNUSED(instance);

    return consumed;
}

const Scene js_app_launcher_scene_setup = {
    .data_size = sizeof(JsAppLauncherSceneSetup),
    .enter_callback = js_app_launcher_scene_setup_on_enter,
    .exit_callback = js_app_launcher_scene_setup_on_exit,
    .event_callback = js_app_launcher_scene_setup_on_event,
};
