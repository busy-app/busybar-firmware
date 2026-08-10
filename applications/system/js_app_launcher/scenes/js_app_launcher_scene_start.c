#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/dialog.h>
#include <gui/modules/menu.h>

#include <storage/storage.h>

#define COLOR_START ((Color)COLOR_MAKE_HEX(0x22C55E))
#define COLOR_SETUP ((Color)COLOR_MAKE_HEX(0xFF6D16))

typedef enum {
    JsAppLauncherSceneStartMenuIdxStart,
    JsAppLauncherSceneStartMenuIdxSetup,
    JsAppLauncherSceneStartMenuIdxMax,
} JsAppLauncherSceneStartMenuIdx;

typedef struct {
    Dialog* front_dialog;
    Menu* back_menu;
} JsAppLauncherSceneStart;

static void js_app_launcher_scene_start_menu_callback(uint8_t index, void* context) {
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

    JsAppInfo info;
    furi_check(js_app_get_info(instance->js_app, &info));

    with_gui(instance->gui, {
        data->front_dialog = dialog_alloc(instance->front_window);
        dialog_set_icon(data->front_dialog, info.path.icon.front);
        dialog_set_text(data->front_dialog, info.manifest.name);
        dialog_set_options(data->front_dialog, "Start", "Setup");
        dialog_set_option_colors(data->front_dialog, COLOR_START, COLOR_SETUP);
        dialog_set_callback(
            data->front_dialog, js_app_launcher_scene_start_menu_callback, instance);

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
        dialog_free(data->front_dialog);
        menu_free(data->back_menu);
    });
}

static bool js_app_launcher_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    JsAppLauncher* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == JsAppLauncherSceneStartMenuIdxStart) {
            scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdRun);
        } else if(event->event == JsAppLauncherSceneStartMenuIdxSetup) {
            scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdSetup);
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
