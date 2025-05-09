#include "../busy.h"

#include <gui/modules/label.h>

#include "../widgets/progress_view.h"

#define DONE_TRANSITION_DELAY_MS (4000)
#define REST_TRANSITION_DELAY_MS (1000)

typedef struct {
    Label* front_label;
    ProgressView* front_progress_view;
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

    const BusyTimerState state = busy_timer_get_state(instance->busy_timer);
    uint32_t run_later_delay;

    if(state == BusyTimerStateRest || state == BusyTimerStateIdle) {
        BusyTimerCycles cycles;
        busy_timer_get_cycles(instance->busy_timer, &cycles);

        with_gui(instance->gui, {
            data->front_progress_view = progress_view_alloc(instance->front_window);

            progress_view_set_progress(
                data->front_progress_view, cycles.done_count, cycles.total_count);
        });

        run_later_delay = DONE_TRANSITION_DELAY_MS;

    } else if(state == BusyTimerStateWork) {
        with_gui(instance->gui, {
            data->front_label = label_alloc(instance->front_window);

            label_set_text(data->front_label, "v REST");
            widget_set_align(label_get_base(data->front_label), AlignCenter);
        });

        run_later_delay = REST_TRANSITION_DELAY_MS;

    } else {
        furi_crash();
    }

    run_later(
        instance->event_loop, busy_scene_progress_run_later_callback, instance, run_later_delay);

    busy_start_transition(instance);
}

static void busy_scene_progress_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    busy_prepare_transition(instance, BusyTransitionTypeBlack);

    with_gui(instance->gui, {
        if(data->front_progress_view) {
            progress_view_free(data->front_progress_view);
            data->front_progress_view = NULL;
        }

        if(data->front_label) {
            label_free(data->front_label);
            data->front_label = NULL;
        }
    });
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
