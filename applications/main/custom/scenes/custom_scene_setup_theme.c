#include "../custom.h"

#include <gui/modules/label.h>

typedef struct {
    Label* front_label;
    Label* back_label;
} CustomSceneSetupTheme;

static void custom_scene_setup_theme_on_enter(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneSetupTheme* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->front_label = label_alloc(instance->front_window);
        label_set_text(data->front_label, "Not Implemented");
        widget_set_align(label_get_base(data->front_label), AlignCenter);

        data->back_label = label_alloc(instance->back_window);
        label_set_text(data->back_label, "Not Implemented");
        widget_set_align(label_get_base(data->back_label), AlignCenter);
    });
}

static void custom_scene_setup_theme_on_exit(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneSetupTheme* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        label_free(data->front_label);
        label_free(data->back_label);
    });
}

static bool custom_scene_setup_theme_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    CustomApp* instance = context;

    if(event->type == SceneManagerEventTypeBack) {
        custom_pop_location(instance);
    }

    return consumed;
}

const Scene custom_scene_setup_theme = {
    .enter_callback = custom_scene_setup_theme_on_enter,
    .exit_callback = custom_scene_setup_theme_on_exit,
    .event_callback = custom_scene_setup_theme_on_event,
    .data_size = sizeof(CustomSceneSetupTheme),
};
