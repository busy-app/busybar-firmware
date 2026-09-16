#include "../busy_i.h"
#include "../busy_presets.h"

#include "../widgets/pause_overlay.h"
#include "../widgets/timer_indicator.h"
#include "../widgets/timer_label.h"

#include "../helpers/animate.h"

#include <inttypes.h>

#define COUNTDOWN_THRESHOLD_S (3)

#define TIMER_HIDDEN_TIME_MS (S_TO_MS(15))
#define TIMER_SHOWN_TIME_MS  (S_TO_MS(5))

#define TRANSITION_CLEAR_TIME_MS (100)

typedef struct {
    TimerIndicator* timer_indicator;
    TimerLabel* timer_label;
    PauseOverlay* pause_overlay;
    FuriPubSub* timer_pubsub;
    FuriPubSubSubscription* timer_sub;
    FuriEventLoopTimer* show_label_timer;
    TimerIndicatorPreset custom_preset;
    BusyTimerMode timer_mode;
    BusyTimerMode prev_timer_mode;
    BusyTimerState timer_state;
    uint32_t time_elapsed_s;
    uint32_t time_remaining_s;
    bool is_custom_theme;
    bool is_paused;
    bool is_force_ended;
    bool is_mode_transition;
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
        data->time_elapsed_s = event->tick.time_elapsed_s;
        data->time_remaining_s = event->tick.time_remaining_s;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);

    } else if(event->type == BusyTimerEventTypeModeChanged) {
        data->prev_timer_mode = data->timer_mode;
        data->timer_mode = event->mode_changed.mode;
        busy_send_custom_event(instance, BusyCustomEventTimerModeChanged);

    } else if(event->type == BusyTimerEventTypeStateChanged) {
        data->timer_state = event->state_changed.state;
        busy_send_custom_event(instance, BusyCustomEventTimerStateChanged);

    } else if(event->type == BusyTimerEventTypeIntervalEnded) {
        data->is_force_ended = event->interval_ended.is_forced;
        busy_send_custom_event(instance, BusyCustomEventTimerIntervalEnded);

    } else if(event->type == BusyTimerEventTypePaused) {
        data->is_paused = event->paused.is_paused;
        busy_send_custom_event(instance, BusyCustomEventTimerPaused);
    }
}

static bool busy_scene_timer_has_label_tweaks(const BusySceneTimer* data) {
    return data->is_custom_theme && data->timer_state == BusyTimerStateWork;
}

static void busy_scene_timer_update_tick(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    const uint32_t time_remain_s = data->time_remaining_s;
    const uint32_t time_elapsed_s = data->time_elapsed_s;

    const float progress = (float)time_elapsed_s / (time_elapsed_s + time_remain_s);

    with_gui(instance->gui, {
        timer_indicator_set_progress(data->timer_indicator, progress);
        timer_label_set_time(data->timer_label, time_remain_s);

        uint32_t h = S_TO_H(time_remain_s);
        uint32_t m = S_TO_M(time_remain_s - H_TO_S(h));
        uint32_t s = time_remain_s - H_TO_S(h) - M_TO_S(m);

        FuriString* mirror_card_footer_text =
            (h > 0) ? furi_string_alloc_printf("%" PRIu32 ":%02" PRIu32 ":%02" PRIu32, h, m, s) :
                      furi_string_alloc_printf("%02" PRIu32 ":%02" PRIu32, m, s);
        mirror_card_set_footer_primary_text(
            instance->timer_card, furi_string_get_cstr(mirror_card_footer_text));
        furi_string_free(mirror_card_footer_text);

        if(busy_scene_timer_has_label_tweaks(data) && time_remain_s == COUNTDOWN_THRESHOLD_S) {
            timer_label_show(data->timer_label, true);
            furi_event_loop_timer_start(data->show_label_timer, TIMER_SHOWN_TIME_MS);
        }
    });

    if(data->timer_mode != BusyTimerModeInfinite) {
        if(time_remain_s == 0) {
            audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_finish.snd"));
        } else if(time_remain_s <= COUNTDOWN_THRESHOLD_S) {
            audio_play_file(instance->audio, BUSY_SOUND_PATH("countdown_tick.snd"));
        }
    }
}

static void busy_scene_timer_update_priority(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);
    bool is_active = data->timer_state != BusyTimerStateIdle;
    busy_set_priority(instance, is_active);
}

static void busy_scene_timer_update_front_display_blanking(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);
    busy_set_front_display_blanking(
        instance, data->timer_state != BusyTimerStateWork || data->is_paused);
}

static void busy_scene_timer_update_timer_mode(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    data->is_mode_transition = (data->prev_timer_mode == BusyTimerModeInfinite);

    with_gui(instance->gui, {
        if(data->timer_mode == BusyTimerModeInfinite) {
            widget_set_visible(timer_label_get_base(data->timer_label), false);
            mirror_card_set_show_footer(instance->timer_card, false);

        } else if(data->timer_mode == BusyTimerModeSimple) {
            widget_set_visible(timer_label_get_base(data->timer_label), true);
            mirror_card_set_show_footer(instance->timer_card, true);

        } else if(data->timer_mode == BusyTimerModeInterval) {
            widget_set_visible(timer_label_get_base(data->timer_label), true);
            mirror_card_set_show_footer(instance->timer_card, true);
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

    if(!busy_scene_timer_has_label_tweaks(data)) {
        if(data->is_mode_transition) {
            ret = &busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeInfToSimple];
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

        if(timer_indicator_transition) {
            Widget* timer_label_base = timer_label_get_base(data->timer_label);

            const int32_t start_pos = widget_get_width(timer_label_base);
            const int32_t end_pos = 0;

            animate_pos_x(
                timer_label_base, start_pos, end_pos, timer_indicator_transition->duration_ms);
        }

        if(timer_label_preset) {
            timer_label_set_preset(data->timer_label, timer_label_preset);

            if(busy_scene_timer_has_label_tweaks(data)) {
                timer_label_enable_background(data->timer_label, true);

                if(!data->is_mode_transition) {
                    timer_label_hide(data->timer_label, false);
                    furi_event_loop_timer_start(data->show_label_timer, TIMER_HIDDEN_TIME_MS);
                }

            } else {
                timer_label_enable_background(data->timer_label, false);
                timer_label_show(data->timer_label, false);
                furi_event_loop_timer_stop(data->show_label_timer);
            }
        }
    });

    busy_scene_timer_update_priority(instance);
    busy_scene_timer_update_front_display_blanking(instance);
}

static void busy_scene_timer_clear_transition(BusyApp* instance) {
    bool was_cleared = false;

    with_gui(instance->gui, {
        Widget* transition_overlay = transition_overlay_get_base(instance->transition_overlay);

        if(widget_is_visible(transition_overlay)) {
            widget_set_visible(transition_overlay, false);
            was_cleared = true;
        }
    });

    if(was_cleared) {
        // Small delay for the Gui to redraw the background
        furi_delay_ms(TRANSITION_CLEAR_TIME_MS);
    }
}

static void busy_scene_timer_handle_pause(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    const bool is_paused = data->is_paused;

    if(is_paused) {
        // NOTE: Special case where the pause overlay is shown immediately
        // after entering the scene (e.g. when restoring the timer from stored snapshot).
        // Needed for the theme background to be visible through the pause overlay.
        busy_scene_timer_clear_transition(instance);
    }

    with_gui(instance->gui, {
        pause_overlay_show(data->pause_overlay, is_paused);
        mirror_card_set_show_header(instance->timer_card, !is_paused);
        timer_indicator_enable_animations(data->timer_indicator, !is_paused);

        if(!is_paused && busy_scene_timer_has_label_tweaks(data)) {
            if(!data->is_mode_transition) {
                furi_event_loop_timer_start(data->show_label_timer, TIMER_HIDDEN_TIME_MS);
                timer_label_hide(data->timer_label, true);
            } else {
                // HACK: BusyTimerEventTypePaused event is the last one to be emitted
                // in busy_timer_notify_initial_state(), reset ongoing transition flag here
                data->is_mode_transition = false;
            }

        } else {
            furi_event_loop_timer_stop(data->show_label_timer);
        }
    });

    busy_scene_timer_update_priority(instance);
    busy_scene_timer_update_front_display_blanking(instance);
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

static void busy_scene_timer_handle_increment_decrement(BusyApp* instance, int32_t value) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(busy_scene_timer_has_label_tweaks(data)) {
        with_gui(instance->gui, {
            timer_label_show(data->timer_label, true);
            furi_event_loop_timer_start(data->show_label_timer, TIMER_SHOWN_TIME_MS);
        });
    }

    busy_timer_add_time(instance->busy_timer, value);
}

static void busy_scene_timer_handle_back(BusyApp* instance) {
    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    if(!data->is_paused) {
        busy_timer_stop(instance->busy_timer);

        busy_prepare_transition(instance, BusyTransitionTypeDefault);

        busy_set_front_display_blanking(instance, false);

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
    busy_set_front_display_blanking(instance, true);

    scene_manager_next_scene(instance->scene_manager, next_scene_id);
}

static void busy_scene_timer_apply_theme(BusyApp* instance) {
    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    const bool is_custom_theme = !busy_theme_is_default(instance->theme);

    if(is_custom_theme) {
        memset(&data->custom_preset, 0, sizeof(TimerIndicatorPreset));

        BusyThemeInfo info;
        busy_theme_get_info(instance->theme, &info);

        const BusyThemeFileType bg_type = info.bg_type;

        if(bg_type == BusyThemeFileTypeImage) {
            data->custom_preset.foreground_config.image_path = info.bg_path;
        } else if(bg_type == BusyThemeFileTypeAnim) {
            data->custom_preset.background_config.anim_path = info.bg_path;
        } else {
            furi_crash("Invalid BusyThemeFileType value");
        }
    }

    data->is_custom_theme = is_custom_theme;
}

static void busy_scene_timer_handle_app_config_changed(BusyApp* instance) {
    busy_scene_timer_apply_theme(instance);
}

static void busy_scene_timer_show_label_timer_callback(void* context) {
    furi_assert(context);
    const BusyApp* instance = context;

    const BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    FuriEventLoopTimer* label_timer = data->show_label_timer;

    const uint32_t prev_interval_ms = furi_event_loop_timer_get_interval(label_timer);
    uint32_t interval_ms;

    with_gui(instance->gui, {
        if(prev_interval_ms == TIMER_HIDDEN_TIME_MS) {
            interval_ms = TIMER_SHOWN_TIME_MS;
            timer_label_show(data->timer_label, true);

        } else if(prev_interval_ms == TIMER_SHOWN_TIME_MS) {
            interval_ms = TIMER_HIDDEN_TIME_MS;
            timer_label_hide(data->timer_label, true);

        } else {
            furi_crash("Illegal timer label interval");
        }
    });

    furi_event_loop_timer_start(label_timer, interval_ms);
}

// Standard SceneManager event handlers

static void busy_scene_timer_on_enter(void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    updater_pause_autoupdates(instance->updater);

    busy_scene_timer_apply_theme(instance);

    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_timer_input_callback, instance);

        data->timer_indicator = timer_indicator_alloc(instance->front_window);
        data->timer_label = timer_label_alloc(instance->front_window);
        data->pause_overlay = pause_overlay_alloc(instance->front_window);

        widget_set_align(timer_label_get_base(data->timer_label), AlignTopRight);
        timer_label_hide(data->timer_label, false);

        widget_set_visible(mirror_card_get_base(instance->timer_card), true);
        mirror_card_set_show_header(instance->timer_card, true);
    });

    data->timer_pubsub = busy_timer_get_pubsub(instance->busy_timer);
    data->timer_sub =
        furi_pubsub_subscribe(data->timer_pubsub, busy_scene_timer_pubsub_callback, instance);

    data->show_label_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_scene_timer_show_label_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);

    data->timer_mode = BusyTimerModeMax;
    data->prev_timer_mode = BusyTimerModeMax;

    if(!instance->show_timer_requested) {
        busy_timer_start(
            instance->busy_timer, busy_get_profile_id(instance), BusyTimerSessionSourceDevice);
    }

    busy_start_transition(instance);
}

static void busy_scene_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    instance->show_timer_requested = false;

    busy_set_priority(instance, false);

    BusySceneTimer* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdTimer);

    furi_event_loop_timer_free(data->show_label_timer);

    furi_pubsub_unsubscribe(data->timer_pubsub, data->timer_sub);

    data->is_force_ended = false;
    data->is_paused = false;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_timer_input_callback);

        mirror_card_set_show_header(instance->timer_card, false);
        mirror_card_set_show_footer(instance->timer_card, false);

        timer_indicator_free(data->timer_indicator);
        timer_label_free(data->timer_label);
        pause_overlay_free(data->pause_overlay);
    });

    updater_resume_autoupdates(instance->updater);
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
            busy_scene_timer_handle_increment_decrement(instance, BUSY_TIMER_TIME_INCREMENT_MN);

        } else if(event->event == BusyCustomEventTimeDecrement) {
            busy_scene_timer_handle_increment_decrement(instance, -BUSY_TIMER_TIME_INCREMENT_MN);

        } else if(event->event == BusyCustomEventReturnToStart) {
            busy_scene_timer_handle_return_to_start(instance);

        } else if(event->event == BusyCustomEventAppConfigChanged) {
            busy_scene_timer_handle_app_config_changed(instance);
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
