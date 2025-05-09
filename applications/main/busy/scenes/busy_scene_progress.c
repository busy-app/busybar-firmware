#include "../busy.h"

#include <gui/modules/label.h>

#define DONE_TRANSITION_DELAY_MS (2000)
#define REST_TRANSITION_DELAY_MS (1000)

typedef struct {
    Label* front_label;
} BusySceneProgress;

static void busy_scene_progress_run_later_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
}

static void busy_scene_progress_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->front_label = label_alloc(instance->front_window);
        widget_set_align(label_get_base(data->front_label), AlignCenter);
    });

    const BusyTimerState state = busy_timer_get_state(instance->busy_timer);

    if(state == BusyTimerStateRest || state == BusyTimerStateIdle) {
        BusyTimerCycles cycles;
        busy_timer_get_cycles(instance->busy_timer, &cycles);

        with_gui(instance->gui, {
            label_set_text_fmt(
                data->front_label, "DONE: %lu/%lu", cycles.done_count, cycles.total_count);
        });

        run_later(
            instance->event_loop,
            busy_scene_progress_run_later_callback,
            instance,
            DONE_TRANSITION_DELAY_MS);

    } else if(state == BusyTimerStateWork) {
        with_gui(instance->gui, { label_set_text(data->front_label, "v REST"); });

        run_later(
            instance->event_loop,
            busy_scene_progress_run_later_callback,
            instance,
            REST_TRANSITION_DELAY_MS);
    }

    busy_start_transition(instance);
}

static void busy_scene_progress_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    busy_prepare_transition(instance, BusyTransitionTypeBlack);

    with_gui(instance->gui, { label_free(data->front_label); });
}

static bool busy_scene_progress_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    UNUSED(instance);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_progress = {
    .enter_callback = busy_scene_progress_on_enter,
    .exit_callback = busy_scene_progress_on_exit,
    .event_callback = busy_scene_progress_on_event,
    .data_size = sizeof(BusySceneProgress),
};
