#include "../apps_menu_i.h"
#include "apps_menu_scenes.h"

#include <gui/modules/image.h>
#include <gui/modules/label.h>

typedef struct {
    Image* front_placeholder;
    Label* front_placeholder_text;

    Image* back_placeholder;
    Label* back_placeholder_text;
} AppsMenuSceneMain;

static void apps_menu_scene_main_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    AppsMenuSceneMain* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        // front:

        scene->front_placeholder = image_alloc(app->front_scene_window);
        image_set_source(scene->front_placeholder, APPS_MENU_IMG("apps_placeholder_front_49x16"));
        Widget* front_placeholder_base = image_get_base(scene->front_placeholder);
        widget_set_align(front_placeholder_base, AlignCenter);

        scene->front_placeholder_text = label_alloc(app->front_scene_window);
        label_set_text(scene->front_placeholder_text, "Coming soon...");
        label_set_text_align(scene->front_placeholder_text, TextAlignCenter);
        Widget* front_placeholder_text_base = label_get_base(scene->front_placeholder_text);
        widget_set_align(front_placeholder_text_base, AlignCenter);

        // back:

        scene->back_placeholder = image_alloc(app->back_scene_window);
        image_set_source(scene->back_placeholder, APPS_MENU_IMG("apps_placeholder_back_98x44"));
        Widget* back_placeholder_base = image_get_base(scene->back_placeholder);
        widget_set_align(back_placeholder_base, AlignTopMid);
        widget_set_padding(back_placeholder_base, 0, 0, 4, 0);

        scene->back_placeholder_text = label_alloc(back_placeholder_base);
        label_set_text(scene->back_placeholder_text, "Coming soon...");
        label_set_text_align(scene->back_placeholder_text, TextAlignCenter);
        Widget* back_placeholder_text_base = label_get_base(scene->back_placeholder_text);
        widget_set_align(back_placeholder_text_base, AlignBottomMid);
    });
}

static void apps_menu_scene_main_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    AppsMenuSceneMain* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        image_free(scene->front_placeholder);
        label_free(scene->front_placeholder_text);
        image_free(scene->back_placeholder);
        label_free(scene->back_placeholder_text);
    });
}

static bool apps_menu_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    UNUSED(app);
    UNUSED(event);

    return false;
}

const Scene apps_menu_scene_main = {
    .enter_callback = apps_menu_scene_main_on_enter,
    .exit_callback = apps_menu_scene_main_on_exit,
    .event_callback = apps_menu_scene_main_on_event,
    .data_size = sizeof(AppsMenuSceneMain),
};
