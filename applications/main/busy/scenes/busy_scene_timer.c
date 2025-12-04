#include "../busy_i.h"
#include "../busy_presets.h"

#include <gui/modules/flex_layout.h>

#include "../widgets/pause_overlay.h"
#include "../widgets/timer_indicator.h"
#include "../widgets/timer_label.h"

#define COUNTDOWN_THRESHOLD_S (3)

#define PROGRESS_TRANSITION_MS (1000)

typedef struct {
    FlexLayout* front_flex;
    TimerIndicator* timer_indicator;
    TimerLabel* timer_label;
    PauseOverlay* pause_overlay;
    RunLater* run_later;
    FuriPubSub* timer_pubsub;
    FuriPubSubSubscription* timer_sub;
    BusyTimerMode timer_mode;
    BusyTimerMode prev_timer_mode;
    BusyTimerTime timer_time;
    BusyTimerState timer_state;
    bool is_paused;
    bool is_force_ended;
} BusySceneTimer;

static bool busy_scene_timer_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            custom_event = BusyCustomEventTimeIncrement;
            consumed = true;

        } else if(event->key == InputKeyDown) {
            custom_event = BusyCustomEventTimeDecrement;
            consumed = true;

        } else if(event->key == InputKeyOk) {
            custom_event = BusyCustomEventTimerSkip;
            consumed = true;

        } else if(event->key == InputKeyStart) {
            custom_event = BusyCustomEventStartShortPressed;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_timer_pubsub_callback(const void* msg, void* context) {
    furi_assert(msg);
    furi_assert(context);

    const BusyTimerEvent* event = msg;

    BusyApp* instance = context;
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time = event->time;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);

    } else if(event->type == BusyTimerEventTypeModeChanged) {
        data->prev_timer_mode = data->timer_mode;
        data->timer_mode = event->mode;
        busy_send_custom_event(instance, BusyCustomEventTimerModeChanged);

    } else if(event->type == BusyTimerEventTypeStateChanged) {
        data->timer_state = event->state;
        busy_send_custom_event(instance, BusyCustomEventTimerStateChanged);

    } else if(event->type == BusyTimerEventTypeIntervalEnded) {
        data->is_force_ended = event->is_force_ended;
        busy_send_custom_event(instance, BusyCustomEventTimerIntervalEnded);

    } else if(event->type == BusyTimerEventTypeTimerPaused) {
        data->is_paused = event->timer_paused.is_paused;
        busy_send_custom_event(instance, BusyCustomEventTimerPaused);
    }
}

static void busy_scene_timer_run_later_callback(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(data->is_force_ended) {
        busy_prepare_transition(instance, BusyTransitionTypeSkip);
    } else if(data->timer_state == BusyTimerStateRest) {
        busy_prepare_transition(instance, BusyTransitionTypeRestDone);
    } else {
        busy_prepare_transition(instance, BusyTransitionTypeWorkDone);
    }

    if(data->timer_mode == BusyTimerModeInterval) {
        scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdProgress);
    } else {
        scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
    }
}

static void busy_scene_timer_update_tick(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);
    const BusyTimerTime* time = &data->timer_time;

    const float progress = (float)time->elapsed_s / (time->elapsed_s + time->remain_s);

    with_gui(instance->gui, {
        timer_indicator_set_progress(data->timer_indicator, progress);
        timer_label_set_time(data->timer_label, data->timer_time.remain_s);
        timer_card_set_time(instance->timer_card, data->timer_time.remain_s);
    });

    if(time->remain_s == 0) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_finish.snd"));
    } else if(time->remain_s <= COUNTDOWN_THRESHOLD_S) {
        audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_tick.snd"));
    }
}

static void busy_scene_timer_update_lights(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(data->is_paused) {
        busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    } else if(data->timer_state == BusyTimerStateWork) {
        busy_set_status_lights(instance, BusyStatusLightsTypeWork);
    } else if(data->timer_state == BusyTimerStateRest) {
        busy_set_status_lights(instance, BusyStatusLightsTypeRest);
    }
}

static void busy_scene_timer_update_matter(BusyApp* app) {
    const BusySceneTimer* scene =
        scene_manager_get_scene_data(app->scene_manager, BusyAppSceneIdTimer);
    busy_set_matter(app, (scene->timer_state == BusyTimerStateWork) && !scene->is_paused);
}

static void busy_scene_timer_update_timer_mode(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    with_gui(instance->gui, {
        if(data->timer_mode == BusyTimerModeInfinite) {
            widget_set_visible(timer_label_get_base(data->timer_label), false);
            timer_card_show_time(instance->timer_card, false);

        } else if(data->timer_mode == BusyTimerModeSimple) {
            widget_set_visible(timer_label_get_base(data->timer_label), true);
            timer_card_show_time(instance->timer_card, true);

        } else if(data->timer_mode == BusyTimerModeInterval) {
            widget_set_visible(timer_label_get_base(data->timer_label), true);
            timer_card_show_time(instance->timer_card, true);
        }
    });
}

static void busy_scene_timer_update_timer_state(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    const TimerIndicatorPreset* preset = NULL;
    const TimerIndicatorTransition* transition = NULL;

    if(data->timer_state == BusyTimerStateWork) {
        if(data->timer_mode == BusyTimerModeInfinite) {
            preset = &busy_timer_indicator_presets[BusyTimerIndicatorTypeWorkBig];
        } else if(data->timer_mode == BusyTimerModeSimple) {
            preset = &busy_timer_indicator_presets[BusyTimerIndicatorTypeWork];
            if(data->prev_timer_mode == BusyTimerModeInfinite) {
                // Special case: transitioning from Infinite to Simple
                transition =
                    &busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeInfToSimple];
            }
        } else if(data->timer_mode == BusyTimerModeInterval) {
            preset = &busy_timer_indicator_presets[BusyTimerIndicatorTypeWork];
            if(data->prev_timer_mode == BusyTimerModeInfinite) {
                // Special case: transitioning from Infinite to Interval
                transition =
                    &busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeInfToSimple];
            }
        }

    } else if(data->timer_state == BusyTimerStateRest) {
        furi_assert(data->timer_mode == BusyTimerModeInterval);
        preset = &busy_timer_indicator_presets[BusyTimerIndicatorTypeRest];
    }

    if(preset) {
        with_gui(instance->gui, {
            timer_indicator_set_preset(data->timer_indicator, preset, transition);
        });
    }

    busy_scene_timer_update_lights(instance);
    busy_scene_timer_update_matter(instance);
}

static void busy_scene_timer_handle_pause(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    with_gui(instance->gui, {
        pause_overlay_show(data->pause_overlay, data->is_paused);
        timer_card_show_header(instance->timer_card, !data->is_paused);
        timer_indicator_enable_animations(data->timer_indicator, !data->is_paused);
    });

    busy_scene_timer_update_lights(instance);
    busy_scene_timer_update_matter(instance);
}

static void busy_scene_timer_handle_skip(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if((data->timer_mode == BusyTimerModeInterval) && !data->is_paused) {
        busy_prepare_transition(instance, BusyTransitionTypeSkip);
        busy_start_transition(instance);
        busy_timer_skip(instance->busy_timer);
    }
}

static void busy_scene_timer_handle_back(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(!data->is_paused) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        if(!busy_return_to_start_scene(instance)) {
            busy_exit(instance);
        }

    } else {
        busy_timer_toggle(instance->busy_timer);
    }
}

static void busy_scene_timer_handle_return_to_start(BusyApp* instance) {
    busy_prepare_transition(instance, BusyTransitionTypeAutomatic);
    furi_check(busy_return_to_start_scene(instance));
}

static void busy_scene_timer_go_to_progress_scene(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(data->is_force_ended) {
        busy_scene_timer_run_later_callback(instance);

    } else {
        furi_assert(data->run_later == NULL);

        data->run_later = run_later(
            instance->event_loop,
            busy_scene_timer_run_later_callback,
            instance,
            PROGRESS_TRANSITION_MS);
    }
}

static void busy_scene_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_timer_input_callback, instance);

        data->front_flex = flex_layout_alloc(instance->front_window, FlexLayoutTypeRow);
        widget_set_pos_x(flex_layout_get_base(data->front_flex), 1);
        flex_layout_set_spacing(data->front_flex, 2);

        data->timer_indicator = timer_indicator_alloc(flex_layout_get_base(data->front_flex));

        data->timer_label = timer_label_alloc(flex_layout_get_base(data->front_flex));
        widget_set_margin(timer_label_get_base(data->timer_label), 0, 0, 1, 0);

        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
        timer_card_show_header(instance->timer_card, true);
    });

    data->timer_pubsub = busy_timer_get_pubsub(instance->busy_timer);
    data->timer_sub =
        furi_pubsub_subscribe(data->timer_pubsub, busy_scene_timer_pubsub_callback, instance);

    data->timer_mode = BusyTimerModeMax;
    data->prev_timer_mode = BusyTimerModeMax;

    if(!instance->show_timer_requested) {
        busy_timer_start(instance->busy_timer);
    }

    busy_start_transition(instance);
}

static void busy_scene_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    instance->show_timer_requested = false;

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);
    busy_set_matter(instance, false);

    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    furi_pubsub_unsubscribe(data->timer_pubsub, data->timer_sub);

    if(data->run_later) {
        run_later_cancel(data->run_later);
        data->run_later = NULL;
    }

    data->is_force_ended = false;
    data->is_paused = false;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_timer_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        flex_layout_free(data->front_flex);
        pause_overlay_free(data->pause_overlay);
    });
}

static bool busy_scene_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            busy_scene_timer_update_tick(instance);

        } else if(event->event == BusyCustomEventTimerModeChanged) {
            busy_scene_timer_update_timer_mode(instance);

        } else if(event->event == BusyCustomEventTimerStateChanged) {
            busy_scene_timer_update_timer_state(instance);

        } else if(event->event == BusyCustomEventTimerIntervalEnded) {
            busy_scene_timer_go_to_progress_scene(instance);

        } else if(event->event == BusyCustomEventTimerPaused) {
            busy_scene_timer_handle_pause(instance);

        } else if(event->event == BusyCustomEventStartShortPressed) {
            busy_timer_toggle(instance->busy_timer);

        } else if(event->event == BusyCustomEventTimerSkip) {
            busy_scene_timer_handle_skip(instance);

        } else if(event->event == BusyCustomEventTimeIncrement) {
            busy_timer_add_time(instance->busy_timer, BUSY_TIMER_TIME_INCREMENT_MN);

        } else if(event->event == BusyCustomEventTimeDecrement) {
            busy_timer_add_time(instance->busy_timer, -BUSY_TIMER_TIME_INCREMENT_MN);

        } else if(event->event == BusyCustomEventReturnToStart) {
            busy_scene_timer_handle_return_to_start(instance);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_scene_timer_handle_back(instance);

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_timer = {
    .enter_callback = busy_scene_timer_on_enter,
    .exit_callback = busy_scene_timer_on_exit,
    .event_callback = busy_scene_timer_on_event,
    .data_size = sizeof(BusySceneTimer),
};
