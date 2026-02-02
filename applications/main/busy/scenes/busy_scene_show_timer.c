#include "../busy_i.h"

static void busy_scene_show_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;

    busy_prepare_transition(instance, BusyTransitionTypeAutomatic);

    if(!scene_manager_search_and_switch_to_previous_scene(
           instance->scene_manager, BusyAppSceneIdTimer)) {
        with_gui(instance->gui, {
            widget_set_visible(nav_bar_get_base(instance->nav_bar), false);
            widget_set_visible(timer_card_get_base(instance->timer_card), true);

            timer_card_show_header(instance->timer_card, false);
            timer_card_show_time(instance->timer_card, false);
        });

        scene_manager_replace_current_scene(instance->scene_manager, BusyAppSceneIdTimer);
    }
}

static void busy_scene_show_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    UNUSED(instance);
}

static bool busy_scene_show_timer_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    bool consumed = true;
    return consumed;
}

const Scene busy_scene_show_timer = {
    .enter_callback = busy_scene_show_timer_on_enter,
    .exit_callback = busy_scene_show_timer_on_exit,
    .event_callback = busy_scene_show_timer_on_event,
};
