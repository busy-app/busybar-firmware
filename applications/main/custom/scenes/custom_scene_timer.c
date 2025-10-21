#include "../custom.h"
#include "../custom_presets.h"

#include <gui/modules/image.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/flex_layout.h>

#include <busy/widgets/pause_overlay.h>
#include <busy/widgets/timer_bar.h>
#include <busy/widgets/timer_label.h>

#define COUNTDOWN_THRESHOLD_S (3)

#define PROGRESS_TRANSITION_MS (1000)

typedef struct {
    FlexLayout* front_flex;
    AnimImage* image;
} CustomSceneTimer;

static bool custom_scene_timer_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    return false;
}

static void custom_scene_timer_handle_back(CustomApp* instance) {
    custom_prepare_transition(instance, CustomTransitionTypeDefault);
    scene_manager_search_and_switch_to_previous_scene(
        instance->scene_manager, CustomAppSceneIdStart);
}

static void custom_scene_timer_on_enter(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, custom_scene_timer_input_callback, instance);

        data->front_flex = flex_layout_alloc(instance->front_window, FlexLayoutTypeRow);
        flex_layout_set_spacing(data->front_flex, 2);

        data->image = anim_image_alloc(flex_layout_get_base(data->front_flex));
        anim_image_set_source(data->image, CUSTOM_ANIM_PATH("keepout_label_72x16.anim"));
        anim_image_set_loop(data->image, true);

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
        timer_card_show_header(instance->timer_card, true);
    });

    custom_set_status_lights(instance, CustomStatusLightsTypeWork);
    custom_set_matter(instance, true);

    custom_start_transition(instance);
}

static void custom_scene_timer_on_exit(void* context) {
    furi_assert(context);

    CustomApp* instance = context;

    custom_set_status_lights(instance, CustomStatusLightsTypeOff);
    custom_set_matter(instance, false);

    CustomSceneTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, custom_scene_timer_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        flex_layout_free(data->front_flex);
    });
}

static bool custom_scene_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    CustomApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        custom_scene_timer_handle_back(instance);

        consumed = true;
    }

    return consumed;
}

const Scene custom_scene_timer = {
    .enter_callback = custom_scene_timer_on_enter,
    .exit_callback = custom_scene_timer_on_exit,
    .event_callback = custom_scene_timer_on_event,
    .data_size = sizeof(CustomSceneTimer),
};
