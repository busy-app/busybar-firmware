#include "../../settings.h"
#include "../../storage_macros.h"

#include <gui/modules/image.h>
#include <gui/modules/label.h>

#include <matter/matter.h>

typedef struct {
    struct {
        Image* cross;
        Label* message;
    } front;
    struct {
        Image* cross;
        Label* message;
    } back;
} SettingsSceneCommissionDone;

static void settings_scene_matter_commission_done_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneCommissionDone* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        /* front */ {
            scene->front.cross = image_alloc(app->front_scene_window);
            image_set_source(scene->front.cross, SETTINGS_IMG_PATH("checkmark_front_8x6.bin"));
            Widget* spinner_base = image_get_base(scene->front.cross);
            widget_set_align(spinner_base, AlignLeftMid);

            scene->front.message = label_alloc(app->front_scene_window);
            label_set_text(scene->front.message, "Connected");
            Widget* message_base = label_get_base(scene->front.message);
            widget_set_align(message_base, AlignLeftMid);
            widget_set_pos(message_base, 10, 0);
        }

        /* back */ {
            scene->back.cross = image_alloc(app->back_scene_window);
            image_set_source(scene->back.cross, SETTINGS_IMG_PATH("checkmark_back_12x10.bin"));
            Widget* spinner_base = image_get_base(scene->back.cross);
            widget_set_align(spinner_base, AlignCenter);
            widget_set_pos(spinner_base, 0, -8);

            scene->back.message = label_alloc(app->back_scene_window);
            label_set_text(scene->back.message, "Connected");
            Widget* message_base = label_get_base(scene->back.message);
            widget_set_align(message_base, AlignCenter);
            widget_set_pos(message_base, 0, 8);
        }
    });
}

static void settings_scene_matter_commission_done_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneCommissionDone* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        // front:
        label_free(scene->front.message);
        image_free(scene->front.cross);
        // back:
        label_free(scene->back.message);
        image_free(scene->back.cross);
    });
}

static bool
    settings_scene_matter_commission_done_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    furi_assert(context);

    SettingsApp* app = context;
    UNUSED(app);

    bool consumed = false;

    return consumed;
}

const Scene settings_scene_matter_commission_done = {
    .enter_callback = settings_scene_matter_commission_done_on_enter,
    .exit_callback = settings_scene_matter_commission_done_on_exit,
    .event_callback = settings_scene_matter_commission_done_on_event,
    .data_size = sizeof(SettingsSceneCommissionDone),
};
