#include "../busy.h"

#include <gui/modules/image.h>

#include "../widgets/progress_view.h"

#define DONE_TRANSITION_DELAY_MS (4000)
#define REST_TRANSITION_DELAY_MS (1500)

typedef struct {
    ProgressView* front_progress_view;
    Image* front_rest_image;
    RunLater* run_later;
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
    BusyStatusLightsType status_lights;

    if(state == BusyTimerStateRest || state == BusyTimerStateIdle) {
        BusyTimerCycles cycles;
        busy_timer_get_cycles(instance->busy_timer, &cycles);

        with_gui(instance->gui, {
            data->front_progress_view = progress_view_alloc(instance->front_window);

            progress_view_set_progress(
                data->front_progress_view, cycles.done_count, cycles.total_count);
        });

        run_later_delay = DONE_TRANSITION_DELAY_MS;
        status_lights = BusyStatusLightsTypeWork;

    } else if(state == BusyTimerStateWork) {
        with_gui(instance->gui, {
            data->front_rest_image = image_alloc(instance->front_window);
            image_set_source(data->front_rest_image, BUSY_IMG_PATH("rest_done_72x16.bin"));
        });

        run_later_delay = REST_TRANSITION_DELAY_MS;
        status_lights = BusyStatusLightsTypeRest;

    } else {
        furi_crash();
    }

    data->run_later = run_later(
        instance->event_loop, busy_scene_progress_run_later_callback, instance, run_later_delay);

    busy_set_status_lights(instance, status_lights);
    busy_start_transition(instance);
}

static void busy_scene_progress_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    run_later_cancel(data->run_later);

    busy_prepare_transition(instance, BusyTransitionTypeAutomatic);
    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    with_gui(instance->gui, {
        if(data->front_progress_view) {
            progress_view_free(data->front_progress_view);
            data->front_progress_view = NULL;
        }

        if(data->front_rest_image) {
            image_free(data->front_rest_image);
            data->front_rest_image = NULL;
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
