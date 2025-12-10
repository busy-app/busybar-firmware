#include "../busy_i.h"

#include "../widgets/progress_view.h"
#include "../widgets/progress_counter.h"

#include "../helpers/animate.h"

#define PROGRESS_SLIDE_POS_Y     (-6)
#define PROGRESS_SLIDE_TIME_MS   (300)
#define NEXT_TRANSITION_DELAY_MS (2000)

typedef struct {
    ProgressCounter* front_progress_counter;
    ProgressView* front_progress_view;
    RunLater* run_later;
} BusySceneProgress;

static void busy_scene_progress_run_later_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    busy_prepare_transition(instance, BusyTransitionTypeAutomatic);
    scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
}

static bool busy_scene_progress_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyStart) {
            custom_event = BusyCustomEventStartShortPressed;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_progress_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdProgress);

    const BusyTimerState timer_state = busy_timer_get_state(instance->busy_timer);

    BusyTimerCycles timer_cycles;
    busy_timer_get_cycles(instance->busy_timer, &timer_cycles);

    const uint32_t curr_interval_idx = timer_cycles.current_idx;
    const uint32_t prev_interval_idx = curr_interval_idx - 1;
    const uint32_t num_cycles = timer_cycles.total_count;

    BusyStatusLightsType status_lights;
    uint32_t num_intervals_done;
    uint32_t num_intervals_total;

    if(timer_state == BusyTimerStateRest || timer_state == BusyTimerStateIdle) {
        status_lights = BusyStatusLightsTypeWork;
        num_intervals_done = (curr_interval_idx + 1) / 2;
        num_intervals_total = num_cycles;

    } else if(timer_state == BusyTimerStateWork) {
        status_lights = BusyStatusLightsTypeRest;
        num_intervals_done = curr_interval_idx / 2;
        num_intervals_total = num_cycles - 1;

    } else {
        furi_crash("Invalid BusyTimerState value");
    }

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_progress_input_callback, instance);

        data->front_progress_counter = progress_counter_alloc(instance->front_window);
        progress_counter_set_values(
            data->front_progress_counter, num_intervals_done, num_intervals_total);
        widget_set_align(progress_counter_get_base(data->front_progress_counter), AlignTopMid);
        widget_set_pos_y(progress_counter_get_base(data->front_progress_counter), 3);

        data->front_progress_view = progress_view_alloc(instance->front_window);
        progress_view_set_progress(
            data->front_progress_view, prev_interval_idx, num_cycles, false);
        widget_set_align(progress_view_get_base(data->front_progress_view), AlignBottomMid);

        animate_pos_y(
            progress_view_get_base(data->front_progress_view),
            PROGRESS_SLIDE_POS_Y,
            0,
            PROGRESS_SLIDE_TIME_MS);
    });

    data->run_later = run_later(
        instance->event_loop,
        busy_scene_progress_run_later_callback,
        instance,
        NEXT_TRANSITION_DELAY_MS);

    busy_set_status_lights(instance, status_lights);
    busy_start_transition(instance);
}

static void busy_scene_progress_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdProgress);

    run_later_cancel(data->run_later);

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_progress_input_callback);

        progress_counter_free(data->front_progress_counter);
        progress_view_free(data->front_progress_view);
    });
}

static bool busy_scene_progress_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventStartShortPressed) {
            busy_prepare_transition(instance, BusyTransitionTypeSkip);
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        if(!busy_return_to_start_scene(instance)) {
            busy_exit(instance);
        }

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
