#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <storage/storage.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/status_view.h>
#include <gui/modules/overlap_fader.h>
#include <gui/modules/image.h>

typedef struct {
    FlexLayout* front_layout;
    StatusView* back_status;
} JsAppLauncherSceneWebSetup;

static void js_app_launcher_scene_web_setup_on_enter(void* context) {
    JsAppLauncher* instance = context;

    JsAppLauncherSceneWebSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdWebSetup);

    with_gui(instance->gui, {
        widget_set_visible(nav_bar_get_base(instance->nav_bar), false);

        data->front_layout = flex_layout_alloc(instance->front_window, FlexLayoutTypeRow);
        flex_layout_set_align(
            data->front_layout, FlexLayoutAlignStart, FlexLayoutAlignCenter, FlexLayoutAlignCenter);

        StatusView* front_status = status_view_alloc(flex_layout_get_base(data->front_layout));
        flex_layout_set_child_widget_grow(
            data->front_layout, status_view_get_base(front_status), 1);
        status_view_set_primary_text(front_status, "Set up via\nhttp://10.0.4.20");
        status_view_set_icon(front_status, SHARED_IMG_PATH("info_front_8x8.image"), false);

        Image* arrow = image_alloc(flex_layout_get_base(data->front_layout));
        image_set_source(arrow, SHARED_IMG_PATH("back_arrow_front_7x7.image"));
        widget_set_size_content(image_get_base(arrow));

        OverlapFader* fader = overlap_fader_alloc(flex_layout_get_base(data->front_layout));
        overlap_fader_align_to(fader, image_get_base(arrow), OverlapFaderSideLeft);
        widget_set_width(overlap_fader_get_base(fader), 10);

        data->back_status = status_view_alloc(instance->back_window);
        status_view_set_primary_text(data->back_status, "Set up via\nhttp://10.0.4.20");
        status_view_set_icon(data->back_status, SHARED_IMG_PATH("info_back_11x11.image"), false);
    });
}

static void js_app_launcher_scene_web_setup_on_exit(void* context) {
    JsAppLauncher* instance = context;

    JsAppLauncherSceneWebSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdWebSetup);

    with_gui(instance->gui, {
        flex_layout_free(data->front_layout);
        status_view_free(data->back_status);

        widget_set_visible(nav_bar_get_base(instance->nav_bar), true);
    });
}

static bool
    js_app_launcher_scene_web_setup_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return false;
}

const Scene js_app_launcher_scene_web_setup = {
    .data_size = sizeof(JsAppLauncherSceneWebSetup),
    .enter_callback = js_app_launcher_scene_web_setup_on_enter,
    .exit_callback = js_app_launcher_scene_web_setup_on_exit,
    .event_callback = js_app_launcher_scene_web_setup_on_event,
};
