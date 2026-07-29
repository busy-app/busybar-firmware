#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/anim_menu.h>
#include <gui/modules/flex_box.h>
#include <gui/modules/image.h>
#include <gui/modules/label.h>
#include <gui/modules/menu.h>

typedef enum {
    JsAppLauncherSceneStartMenuIdxStart,
    JsAppLauncherSceneStartMenuIdxSetup,
    JsAppLauncherSceneStartMenuIdxMax,
} JsAppLauncherSceneStartMenuIdx;

typedef struct {
    FlexBox* front_flex;
    Image* front_icon;
    Label* front_label;
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

    JsAppInfo info;
    furi_check(js_app_get_info(instance->js_app, &info));

    with_gui(instance->gui, {
        data->front_flex = flex_box_alloc(instance->front_window);
        widget_set_align(flex_box_get_base(data->front_flex), AlignLeftMid);
        flex_box_set_flow(data->front_flex, FlexBoxFlowRow);
        flex_box_set_align(data->front_flex, FlexBoxAlignStart, FlexBoxAlignCenter);
        flex_box_set_spacing(data->front_flex, 2);

        data->front_icon = image_alloc(flex_box_get_base(data->front_flex));
        image_set_source(data->front_icon, info.path.icon.front);

        data->front_label = label_alloc(flex_box_get_base(data->front_flex));
        label_set_text(data->front_label, info.manifest.name);

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
        flex_box_free(data->front_flex);
        anim_menu_free(data->front_menu);
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
