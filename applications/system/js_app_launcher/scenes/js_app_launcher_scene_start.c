#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_menu.h>

typedef enum {
    JsAppLauncherSceneStartMenuIdxStart,
    JsAppLauncherSceneStartMenuIdxSetup,
    JsAppLauncherSceneStartMenuIdxMax,
} JsAppLauncherSceneStartMenuIdx;

typedef struct {
    AnimMenu* front_menu;
    Menu* back_menu;
} JsAppLauncherSceneStart;

static void js_app_launcher_scene_start_menu_callback(uint32_t index, void* context) {
    furi_assert(index < JsAppLauncherSceneStartMenuIdxMax);
    furi_assert(context);

    JsAppLauncher* instance = context;
    js_app_launcher_send_custom_event(instance, index);
}

static void js_app_launcher_scene_start_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneStart* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdStart);

    with_gui(instance->gui, {
        data->front_menu = anim_menu_alloc(instance->front_window);
        widget_set_align(anim_menu_get_base(data->front_menu), AlignRightMid);
        anim_menu_set_source(
            data->front_menu,
            SHARED_ANIM_PATH("start_menu_31x16.anim"),
            JsAppLauncherSceneStartMenuIdxMax);
        anim_menu_set_callback(
            data->front_menu, js_app_launcher_scene_start_menu_callback, instance);

        data->back_menu = menu_alloc(instance->back_window);
        menu_add_item(
            data->back_menu, "Start", NULL, SHARED_IMG_PATH("start_11x11.image"), 0, NULL, NULL);
        menu_add_item(
            data->back_menu, "Setup", NULL, SHARED_IMG_PATH("setup_11x11.image"), 0, NULL, NULL);
    });
}

static void js_app_launcher_scene_start_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneStart* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdStart);

    with_gui(instance->gui, {
        anim_menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool js_app_launcher_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == JsAppLauncherSceneStartMenuIdxStart) {
            // TODO: Start the app
            FURI_LOG_D(TAG, "App started...");
        } else if(event->event == JsAppLauncherSceneStartMenuIdxSetup) {
            // TODO: Go to setup
            FURI_LOG_D(TAG, "Setup entered...");
        }

        consumed = true;
    }

    return consumed;
}

const Scene js_app_launcher_scene_start = {
    .data_size = sizeof(JsAppLauncherSceneStart),
    .enter_callback = js_app_launcher_scene_start_on_enter,
    .exit_callback = js_app_launcher_scene_start_on_exit,
    .event_callback = js_app_launcher_scene_start_on_event,
};
