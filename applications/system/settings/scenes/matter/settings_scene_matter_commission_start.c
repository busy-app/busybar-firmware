#include "../../settings.h"
#include "../../storage_macros.h"
#include "matter_scenes_common.h"

#include <gui/modules/image.h>
#include <gui/modules/label.h>

#include <matter/matter.h>

typedef struct {
    struct {
        Image* spinner; // TODO: AnimImage, once the designers give us the animation
        Label* message;
    } front;
    struct {
        Image* spinner; // TODO: AnimImage, once the designers give us the animation
        Label* message;
    } back;
} SettingsSceneCommissionStart;

static void settings_scene_matter_commission_start_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneCommissionStart* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        /* front */ {
            scene->front.spinner = image_alloc(app->front_scene_window);
            image_set_source(scene->front.spinner, SETTINGS_IMG_PATH("spinner_front_7x7.bin"));
            Widget* spinner_base = image_get_base(scene->front.spinner);
            widget_set_align(spinner_base, AlignLeftMid);

            scene->front.message = label_alloc(app->front_scene_window);
            label_set_text(scene->front.message, "Connecting...");
            Widget* message_base = label_get_base(scene->front.message);
            widget_set_align(message_base, AlignLeftMid);
            widget_set_pos(message_base, 10, 0);
        }

        /* back */ {
            scene->back.spinner = image_alloc(app->back_scene_window);
            image_set_source(scene->back.spinner, SETTINGS_IMG_PATH("spinner_back_16x16.bin"));
            Widget* spinner_base = image_get_base(scene->back.spinner);
            widget_set_align(spinner_base, AlignCenter);
            widget_set_pos(spinner_base, 0, -8);

            scene->back.message = label_alloc(app->back_scene_window);
            label_set_text(scene->back.message, "Connecting...");
            Widget* message_base = label_get_base(scene->back.message);
            widget_set_align(message_base, AlignCenter);
            widget_set_pos(message_base, 0, 8);
        }
    });
}

static void settings_scene_matter_commission_start_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneCommissionStart* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        // front:
        label_free(scene->front.message);
        image_free(scene->front.spinner);
        // back:
        label_free(scene->back.message);
        image_free(scene->back.spinner);
    });
}

static bool
    settings_scene_matter_commission_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* app = context;
    UNUSED(app);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        consumed = matter_scene_replace_current(app, event->event);
    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene settings_scene_matter_commission_start = {
    .enter_callback = settings_scene_matter_commission_start_on_enter,
    .exit_callback = settings_scene_matter_commission_start_on_exit,
    .event_callback = settings_scene_matter_commission_start_on_event,
    .data_size = sizeof(SettingsSceneCommissionStart),
};
