#include "../apps_menu_i.h"
#include "../storage_macros.h"
#include "apps_menu_scenes.h"

#include <gui/modules/image.h>
#include <gui/modules/label.h>
#include <gui/modules/status_view.h>

typedef struct {
    Image* front_image;
    StatusView* back_status_view;
} AppsMenuSceneComingSoon;

static void apps_menu_scene_coming_soon_on_enter(void* context) {
    furi_assert(context);

    AppsMenu* instance = context;
    AppsMenuSceneComingSoon* scene =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdComingSoon);

    with_gui(instance->gui, {
        scene->front_image = image_alloc(instance->front_scene_window);
        image_set_source(scene->front_image, APPS_MENU_IMG_PATH("coming_soon_front_72x16.image"));

        Label* front_label = label_alloc(image_get_base(scene->front_image));
        label_set_text(front_label, "More apps soon");
        label_set_font(front_label, FONT_BUSY_REGULAR_5);
        widget_set_align(label_get_base(front_label), AlignCenter);

        scene->back_status_view = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(scene->back_status_view, SHARED_IMG_PATH("error_back_11x11.image"));
        status_view_set_primary_text(scene->back_status_view, "More apps soon");
        status_view_set_auxiliary_text(
            scene->back_status_view, "Keep your device up to date\nfor upcoming apps");
        widget_set_padding(status_view_get_base(scene->back_status_view), 0, 0, 3, 0);

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);
    });
}

static void apps_menu_scene_coming_soon_on_exit(void* context) {
    furi_assert(context);

    AppsMenu* instance = context;
    AppsMenuSceneComingSoon* scene =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdComingSoon);

    with_gui(instance->gui, {
        image_free(scene->front_image);
        status_view_free(scene->back_status_view);
    });
}

static bool apps_menu_scene_coming_soon_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return false;
}

const Scene apps_menu_scene_coming_soon = {
    .enter_callback = apps_menu_scene_coming_soon_on_enter,
    .exit_callback = apps_menu_scene_coming_soon_on_exit,
    .event_callback = apps_menu_scene_coming_soon_on_event,
    .data_size = sizeof(AppsMenuSceneComingSoon),
};
