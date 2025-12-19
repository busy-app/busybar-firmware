#include "../busy_i.h"
#include "../busy_presets.h"

#include "../widgets/pause_overlay.h"
#include "../widgets/timer_indicator.h"
#include "../widgets/timer_label.h"

#define COUNTDOWN_THRESHOLD_S (3)

#define TIMER_HIDDEN_TIME_S (15)
#define TIMER_SHOWN_TIME_S  (5)

typedef struct {
    TimerIndicator* timer_indicator;
    TimerLabel* timer_label;
    PauseOverlay* pause_overlay;
    FuriPubSub* timer_pubsub;
    FuriPubSubSubscription* timer_sub;
    TimerIndicatorPreset custom_preset;
    BusyTimerMode timer_mode;
    BusyTimerMode prev_timer_mode;
    BusyTimerTime timer_time;
    BusyTimerState timer_state;
    uint32_t prev_label_show_time;
    bool is_custom_theme;
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

static bool busy_scene_timer_has_label_tweaks(const BusySceneTimer* data) {
    return data->is_custom_theme && data->timer_state == BusyTimerStateWork;
}

static void busy_scene_timer_update_tick(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);
    const BusyTimerTime* time = &data->timer_time;

    const uint32_t time_remain_s = time->remain_s;
    const uint32_t time_elapsed_s = time->elapsed_s;

    const float progress = (float)time_elapsed_s / (time_elapsed_s + time_remain_s);

    with_gui(instance->gui, {
        timer_indicator_set_progress(data->timer_indicator, progress);
        timer_label_set_time(data->timer_label, time_remain_s);
        timer_card_set_time(instance->timer_card, time_remain_s);

        if(busy_scene_timer_has_label_tweaks(data)) {
            const uint32_t dt_s = time_elapsed_s - data->prev_label_show_time;

            if(dt_s == TIMER_HIDDEN_TIME_S) {
                timer_label_show(data->timer_label);
            } else if(dt_s == TIMER_HIDDEN_TIME_S + TIMER_SHOWN_TIME_S || dt_s == 0) {
                timer_label_hide(data->timer_label);
                data->prev_label_show_time = time_elapsed_s;
            }
        }
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

static const TimerIndicatorPreset*
    busy_scene_timer_get_indicator_preset(const BusySceneTimer* data) {
    const TimerIndicatorPreset* ret = NULL;

    const BusyTimerState timer_state = data->timer_state;
    const BusyTimerMode timer_mode = data->timer_mode;

    if(timer_state == BusyTimerStateWork) {
        if(data->is_custom_theme) {
            ret = &data->custom_preset;
        } else {
            if(timer_mode == BusyTimerModeInfinite) {
                ret = &busy_timer_indicator_presets[BusyTimerIndicatorTypeWorkBig];
            } else if(timer_mode == BusyTimerModeSimple || timer_mode == BusyTimerModeInterval) {
                ret = &busy_timer_indicator_presets[BusyTimerIndicatorTypeWork];
            }
        }

    } else if(timer_state == BusyTimerStateRest) {
        ret = &busy_timer_indicator_presets[BusyTimerIndicatorTypeRest];
    }

    return ret;
}

static const TimerIndicatorTransition*
    busy_scene_timer_get_indicator_transition(const BusySceneTimer* data) {
    const TimerIndicatorTransition* ret = NULL;

    const BusyTimerMode timer_mode = data->timer_mode;

    if(data->timer_state == BusyTimerStateWork) {
        if(timer_mode == BusyTimerModeSimple || timer_mode == BusyTimerModeInterval) {
            if(data->prev_timer_mode == BusyTimerModeInfinite) {
                ret =
                    &busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeInfToSimple];
            }
        }
    }

    return ret;
}

static const TimerLabelPreset* busy_scene_timer_get_label_preset(const BusySceneTimer* data) {
    const TimerLabelPreset* ret = NULL;

    const BusyTimerState timer_state = data->timer_state;

    if(timer_state == BusyTimerStateWork) {
        ret = &busy_timer_label_presets[BusyTimerLabelTypeWork];
    } else if(timer_state == BusyTimerStateRest) {
        ret = &busy_timer_label_presets[BusyTimerLabelTypeRest];
    }

    return ret;
}

static void busy_scene_timer_update_timer_state(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    data->prev_label_show_time = 0;

    const TimerIndicatorPreset* timer_indicator_preset =
        busy_scene_timer_get_indicator_preset(data);
    const TimerIndicatorTransition* timer_indicator_transition =
        busy_scene_timer_get_indicator_transition(data);
    const TimerLabelPreset* timer_label_preset = busy_scene_timer_get_label_preset(data);

    with_gui(instance->gui, {
        if(timer_indicator_preset) {
            timer_indicator_set_preset(
                data->timer_indicator, timer_indicator_preset, timer_indicator_transition);
        }

        if(timer_label_preset) {
            timer_label_set_preset(data->timer_label, timer_label_preset);
            timer_label_enable_background(
                data->timer_label, busy_scene_timer_has_label_tweaks(data));
        }
    });

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

static void busy_scene_timer_handle_interval_ended(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    BusyTransitionType transition_type;

    if(data->is_force_ended) {
        transition_type = BusyTransitionTypeSkip;
    } else if(data->timer_state == BusyTimerStateRest) {
        transition_type = BusyTransitionTypeRestDone;
    } else {
        transition_type = BusyTransitionTypeWorkDone;
    }

    BusyAppSceneId next_scene_id;

    if(data->timer_mode == BusyTimerModeInterval) {
        if(data->timer_state == BusyTimerStateIdle) {
            next_scene_id = BusyAppSceneIdEnding;
        } else {
            next_scene_id = BusyAppSceneIdProgress;
        }

    } else {
        next_scene_id = BusyAppSceneIdFinish;
    }

    busy_prepare_transition(instance, transition_type);
    scene_manager_next_scene(instance->scene_manager, next_scene_id);
}

static void busy_scene_timer_apply_theme(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    const bool is_custom_theme = !busy_theme_is_default(instance->theme);

    if(is_custom_theme) {
        BusyThemeInfo info;
        busy_theme_get_info(instance->theme, &info);

        const BusyThemeFileType bg_type = info.bg_type;

        if(bg_type == BusyThemeFileTypeImage) {
            data->custom_preset.foreground_config.image_path = info.bg_path;
        } else if(bg_type == BusyThemeFileTypeAnimImage) {
            data->custom_preset.background_config.anim_path = info.bg_path;
        } else if(bg_type == BusyThemeFileTypeLottieAnim) {
            data->custom_preset.progress_config.lottie_path = info.bg_path;
        } else {
            furi_crash("Invalid BusyThemeFileType value");
        }
    }

    data->is_custom_theme = is_custom_theme;
}

// Standard SceneManager event handlers

static void busy_scene_timer_on_enter(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    busy_scene_timer_apply_theme(instance);

    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_timer_input_callback, instance);

        data->timer_indicator = timer_indicator_alloc(instance->front_window);
        data->timer_label = timer_label_alloc(instance->front_window);
        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        widget_set_align(timer_label_get_base(data->timer_label), AlignRightMid);

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

    data->is_force_ended = false;
    data->is_paused = false;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_timer_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        timer_indicator_free(data->timer_indicator);
        timer_label_free(data->timer_label);
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
            busy_scene_timer_handle_interval_ended(instance);

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
