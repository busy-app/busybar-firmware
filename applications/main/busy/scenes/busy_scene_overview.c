#include "../busy.h"

#include <gui/modules/anim_image.h>

#include "../widgets/overview_label.h"

typedef struct {
    AnimImage* front_bg_anim;
    OverviewLabel* front_overview_label;
} BusySceneOverview;

static void busy_scene_overview_run_later_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    busy_prepare_transition(instance, BusyTransitionTypeBlack);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimer);
}

static void busy_scene_overview_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneOverview* data = scene_manager_get_current_scene_data(instance->scene_manager);

    BusyTimerConfig timer_config;
    busy_timer_get_config(instance->busy_timer, &timer_config);

    with_gui(instance->gui, {
        data->front_bg_anim = anim_image_alloc(instance->front_window);
        anim_image_set_source(data->front_bg_anim, BUSY_ANIM_PATH("A_overview_41x16.anim"));
        anim_image_set_loop(data->front_bg_anim, false);

        data->front_overview_label = overview_label_alloc(instance->front_window);
        overview_label_set_intervals(
            data->front_overview_label, timer_config.work_time_mn, timer_config.rest_time_mn);

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
    });

    run_later(instance->event_loop, busy_scene_overview_run_later_callback, instance, 2250);
    busy_start_transition(instance);
}

static void busy_scene_overview_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneOverview* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        anim_image_free(data->front_bg_anim);
        overview_label_free(data->front_overview_label);
    });
}

static bool busy_scene_overview_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;
    UNUSED(instance);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_overview = {
    .enter_callback = busy_scene_overview_on_enter,
    .exit_callback = busy_scene_overview_on_exit,
    .event_callback = busy_scene_overview_on_event,
    .data_size = sizeof(BusySceneOverview),
};
