#include "../busy_i.h"

#define WAIT_ANIM_BEGIN (0)
#define WAIT_ANIM_END   (179)

#define PRESS_ANIM_BEGIN (180)
#define PRESS_ANIM_END   (185)

#include "../widgets/progress_view.h"

typedef struct {
    ProgressView* front_progress_view;
    BusyTimerState timer_state;
} BusySceneNext;

static bool busy_scene_next_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->key == InputKeyStart) {
        if(event->type == InputTypePress) {
            custom_event = BusyCustomEventStartPressed;
            consumed = true;

        } else if(event->type == InputTypeRelease) {
            custom_event = BusyCustomEventStartReleased;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_next_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

    BusyTimerCycles timer_cycles;
    busy_timer_get_cycles(instance->busy_timer, &timer_cycles);

    const uint32_t prev_interval_idx = timer_cycles.current_idx - 1;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_next_input_callback, instance);

        data->front_progress_view = progress_view_alloc(instance->front_window);
        progress_view_set_progress(
            data->front_progress_view, prev_interval_idx, timer_cycles.total_count, true);
        widget_set_align(progress_view_get_base(data->front_progress_view), AlignBottomMid);
    });

    const BusyTimerState timer_state = busy_timer_get_state(instance->busy_timer);

    if(timer_state == BusyTimerStateIdle) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("session_completed.snd"));
    }

    data->timer_state = timer_state;

    busy_start_transition(instance);
}

static void busy_scene_next_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneNext* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_next_input_callback);

        progress_view_free(data->front_progress_view);
    });
}

static bool busy_scene_next_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);

    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        const BusySceneNext* data =
            scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdNext);

        if(event->event == BusyCustomEventStartPressed) {
        } else if(event->event == BusyCustomEventStartReleased) {
            BusyAppSceneId scene_id;
            BusyTransitionType transition_type;

            const BusyTimerState timer_state = data->timer_state;

            if(timer_state == BusyTimerStateIdle) {
                scene_id = BusyAppSceneIdStart;
                transition_type = BusyTransitionTypeDefault;

            } else {
                scene_id = BusyAppSceneIdTimer;
                transition_type = (timer_state == BusyTimerStateWork) ? BusyTransitionTypeWork :
                                                                        BusyTransitionTypeRest;
            }

            busy_prepare_transition(instance, transition_type);
            scene_manager_search_and_switch_to_previous_scene(instance->scene_manager, scene_id);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);
        if(!busy_return_to_start_scene(instance)) {
            busy_exit(instance);
        }

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_next = {
    .enter_callback = busy_scene_next_on_enter,
    .exit_callback = busy_scene_next_on_exit,
    .event_callback = busy_scene_next_on_event,
    .data_size = sizeof(BusySceneNext),
};
