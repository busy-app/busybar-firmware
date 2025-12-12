#include "../busy_i.h"

#include "../widgets/summary_view.h"
#include "../helpers/run_later.h"

typedef struct {
    SummaryView* front_summary;
    RunLater* run_later;
} BusySceneEndig;

static void busy_scene_ending_run_later_callback(void* context) {
    furi_assert(context);

    BusyApp* instance = context;

    busy_prepare_transition(instance, BusyTransitionTypeAutomatic);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdFinish);
}

static void busy_scene_ending_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    BusyTimerConfig config;
    busy_timer_get_config(instance->busy_timer, &config);

    with_gui(instance->gui, {
        data->front_summary = summary_view_alloc(instance->front_window);
        summary_view_set_cycles_count(data->front_summary, config.cycle_count);
        widget_set_align(summary_view_get_base(data->front_summary), AlignCenter);
    });

    data->run_later =
        run_later(instance->event_loop, busy_scene_ending_run_later_callback, instance, 5000);

    busy_start_transition(instance);
}

static void busy_scene_ending_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneEndig* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdEnding);

    run_later_cancel(data->run_later);

    with_gui(instance->gui, { summary_view_free(data->front_summary); });
}

static bool busy_scene_ending_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    UNUSED(instance);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_ending = {
    .enter_callback = busy_scene_ending_on_enter,
    .exit_callback = busy_scene_ending_on_exit,
    .event_callback = busy_scene_ending_on_event,
    .data_size = sizeof(BusySceneEndig),
};
