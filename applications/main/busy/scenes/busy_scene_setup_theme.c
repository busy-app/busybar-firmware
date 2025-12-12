#include "../busy_i.h"

#include <gui/modules/label.h>

typedef struct {
    Label* front_label;
    Label* back_label;
} BusySceneSetupTheme;

static void busy_scene_setup_theme_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);

    with_gui(instance->gui, {
        data->front_label = label_alloc(instance->front_window);
        label_set_text(data->front_label, "Not Implemented");
        widget_set_align(label_get_base(data->front_label), AlignCenter);

        data->back_label = label_alloc(instance->back_window);
        label_set_text(data->back_label, "Not Implemented");
        widget_set_align(label_get_base(data->back_label), AlignCenter);
    });
}

static void busy_scene_setup_theme_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);

    with_gui(instance->gui, {
        label_free(data->front_label);
        label_free(data->back_label);
    });
}

static bool busy_scene_setup_theme_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    BusyApp* instance = context;

    if(event->type == SceneManagerEventTypeBack) {
        busy_pop_location(instance);
    }

    return consumed;
}

const Scene busy_scene_setup_theme = {
    .enter_callback = busy_scene_setup_theme_on_enter,
    .exit_callback = busy_scene_setup_theme_on_exit,
    .event_callback = busy_scene_setup_theme_on_event,
    .data_size = sizeof(BusySceneSetupTheme),
};
