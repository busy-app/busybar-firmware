#include "../busy.h"
#include "../widgets/nav_header.h"

#include <gui/modules/label.h>
#include <gui/modules/flex_layout.h>

typedef struct {
    Label* front_label;
    FlexLayout* back_layout;
    NavHeader* back_header;
    Label* back_label;
} BusySceneSetupTheme;

static void busy_scene_setup_theme_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->front_label = label_alloc(instance->front_window);
        label_set_text(data->front_label, "Not Implemented");
        widget_set_align(label_get_base(data->front_label), AlignCenter);

        data->back_layout = flex_layout_alloc(instance->back_window, FlexLayoutTypeColumn);

        data->back_header = nav_header_alloc(flex_layout_get_base(data->back_layout));
        nav_header_set_image(data->back_header, (const void*)&I_header_busy_39x16);
        nav_header_push_location(data->back_header, "SETUP");
        nav_header_push_location(data->back_header, "THEME");

        data->back_label = label_alloc(flex_layout_get_base(data->back_layout));
        label_set_text(data->back_label, "Not Implemented");
    });
}

static void busy_scene_setup_theme_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        label_free(data->front_label);
        flex_layout_free(data->back_layout);
    });
}

static bool busy_scene_setup_theme_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    UNUSED(instance);

    bool consumed = false;
    return consumed;
}

const Scene busy_scene_setup_theme = {
    .enter_callback = busy_scene_setup_theme_on_enter,
    .exit_callback = busy_scene_setup_theme_on_exit,
    .event_callback = busy_scene_setup_theme_on_event,
    .data_size = sizeof(BusySceneSetupTheme),
};
