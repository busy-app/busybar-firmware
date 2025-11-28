#include "../busy_i.h"

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

    const BusyTimerState state = busy_timer_get_state(instance->busy_timer);

    uint32_t run_later_delay;
    BusyStatusLightsType status_lights;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_progress_input_callback, instance);
    });

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
    BusySceneProgress* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdProgress);

    run_later_cancel(data->run_later);

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_progress_input_callback);

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

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventStartShortPressed) {
            busy_prepare_transition(instance, BusyTransitionTypeSkip);
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        if(!scene_manager_search_and_switch_to_previous_scene(
               instance->scene_manager, BusyAppSceneIdStart)) {
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
