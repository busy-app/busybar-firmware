#include "busy_timer_i.h"

typedef void (*const BusyTimerMessageHandler)(BusyTimer* instance, BusyTimerMessageData* data);

static const BusyTimerMessageHandler busy_timer_message_handlers[];

static const BusyTimerConfig busy_timer_config_default = {
    .work_time_mn = WORK_TIME_DEFAULT_MN,
    .rest_time_mn = REST_TIME_DEFAULT_MN,
    .cycle_count = CYCLE_COUNT_DEFAULT,
    .enable_intervals = ENABLE_INTERVALS_DEFAULT,
    .enable_autostart = ENABLE_AUTOSTART_DEFAULT,
    .enable_sound = ENABLE_SOUND_DEFAULT,
    .enable_speed = ENABLE_SPEED_DEFAULT,
};

// void busy_timer_pause(BusyApp* instance) {
//     furi_event_loop_timer_stop(instance->busy_timer);
//
//     FURI_LOG_I(TAG, "Timer paused");
// }

// void busy_timer_resume(BusyApp* instance) {
//     const uint32_t timeout_ms = instance->enable_speed ? S_TO_MS(1) / SPEED_MULTIPLIER :
//                                                          S_TO_MS(1);
//     furi_event_loop_timer_start(instance->busy_timer, timeout_ms);
//     busy_send_custom_event(instance, instance->state);
//
//     FURI_LOG_I(TAG, "Timer resumed");
// }

// void busy_timer_toggle(BusyApp* instance) {
//     FURI_LOG_I(TAG, "Timer toggle");
//
//     if(busy_timer_is_running(instance)) {
//         busy_timer_pause(instance);
//     } else {
//         busy_timer_resume(instance);
//     }
// }

static const char* busy_timer_get_state_name(BusyTimerState state) {
    furi_assert(state < BusyTimerStateMax);

    static const char* state_names[BusyTimerStateMax] = {
        [BusyTimerStateIdle] = "Idle",
        [BusyTimerStateWork] = "Work",
        [BusyTimerStateRest] = "Rest",
    };

    return state_names[state];
}

static void busy_timer_notify_tick(BusyTimer* instance) {
    FURI_LOG_D(TAG, "Remaining time: %lu s", instance->time_left_s);

    if(instance->callback) {
        const BusyTimerEvent event = {
            .type = BusyTimerEventTypeTick,
            .time_s = instance->time_left_s,
        };

        instance->callback(&event, instance->callback_context);
    }
}

static void busy_timer_notify_state_changed(BusyTimer* instance) {
    FURI_LOG_D(TAG, "State changed: %s", busy_timer_get_state_name(instance->state));

    if(instance->callback) {
        const BusyTimerEvent event = {
            .type = BusyTimerEventTypeStateChanged,
            .state = instance->state,
        };

        instance->callback(&event, instance->callback_context);
    }
}

static BusyTimerState busy_timer_calc_state(BusyTimer* instance) {
    BusyTimerState state;

    if(instance->state == BusyTimerStateIdle) {
        state = BusyTimerStateWork;
    } else if(instance->state == BusyTimerStateWork) {
        state = BusyTimerStateRest;
    } else if(instance->state == BusyTimerStateRest) {
        state = BusyTimerStateWork;
    } else {
        furi_crash();
    }

    return state;
}

static uint32_t busy_timer_calc_time_left(BusyTimer* instance) {
    uint32_t interval_s;

    if(instance->state == BusyTimerStateWork) {
        interval_s = M_TO_S(instance->config.work_time_mn);
    } else if(instance->state == BusyTimerStateRest) {
        interval_s = M_TO_S(instance->config.rest_time_mn);
    } else {
        furi_crash();
    }

    return interval_s;
}

// static BusyCustomEvent busy_timer_calc_event(BusyApp* instance, bool skip_event) {
//     BusyCustomEvent event = BusyCustomEventUpdate;
//
//     if(!skip_event) {
//         if(instance->state == BusyTimerStateIdle) {
//             if(!instance->enable_autorestart_session) {
//                 event = BusyCustomEventSessionEnd;
//             } else {
//                 FURI_LOG_D(TAG, "Autorestart session OFF, skipping event");
//             }
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             if(!instance->enable_autostart_work) {
//                 event = BusyCustomEventIntervalEnd;
//             } else {
//                 FURI_LOG_D(TAG, "Autorestart work OFF, skipping event");
//             }
//         } else if(instance->state == BusyTimerStateWork) {
//             if(!instance->enable_autostart_rest) {
//                 event = BusyCustomEventIntervalEnd;
//             } else {
//                 FURI_LOG_D(TAG, "Autorestart rest OFF, skipping event");
//             }
//         }
//     } else {
//         FURI_LOG_D(TAG, "Skipping state change event");
//     }
//
//     return event;
// }
//
// static void busy_play_finished_sound(BusyApp* instance) {
//     if(instance->enable_sound) {
//         if(instance->state == BusyTimerStateWork) {
//             audio_play_file(instance->audio, EXT_PATH("audio/work_finished.snd"));
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             audio_play_file(instance->audio, EXT_PATH("audio/rest_finished.snd"));
//         }
//     }
// }
//
// static void busy_play_countdown_sound(BusyApp* instance) {
//     if(instance->enable_sound && instance->interval_time_left_s <= 4) {
//         if(instance->state == BusyTimerStateWork) {
//             audio_play_file(instance->audio, EXT_PATH("audio/work_countdown.snd"));
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             audio_play_file(instance->audio, EXT_PATH("audio/rest_countdown.snd"));
//         }
//     }
// }

void busy_timer_next_state(BusyTimer* instance, bool skip_event) {
    UNUSED(skip_event);
    FURI_LOG_I(TAG, "Current state: %s", busy_timer_get_state_name(instance->state));

    instance->state = busy_timer_calc_state(instance);
    instance->time_left_s = busy_timer_calc_time_left(instance);

    const uint32_t timeout_ms = instance->config.enable_speed ? S_TO_MS(1) / SPEED_MULTIPLIER :
                                                                S_TO_MS(1);
    furi_event_loop_timer_start(instance->timer, timeout_ms);

    busy_timer_notify_state_changed(instance);

    if(instance->state != BusyTimerStateIdle) {
        busy_timer_notify_tick(instance);
    }
}

static void busy_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;

    if(instance->time_left_s) {
        instance->time_left_s--;
        busy_timer_notify_tick(instance);

    } else {
        busy_timer_next_state(instance, false);
    }
}

static void busy_timer_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyTimer* instance = context;
    furi_assert(instance->message_queue == object);

    BusyTimerMessage message;
    while(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk) {
        furi_assert(message.type < BusyTimerMessageTypeMax);

        busy_timer_message_handlers[message.type](instance, &message.data);
        api_lock_unlock(message.lock);
    }
}

static int32_t busy_timer_thread(void* arg) {
    furi_assert(arg);
    BusyTimer* instance = arg;

    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, busy_timer_callback, FuriEventLoopTimerTypePeriodic, instance);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        busy_timer_message_queue_callback,
        instance);

    furi_event_loop_run(instance->event_loop);

    furi_event_loop_unsubscribe(instance->event_loop, instance->message_queue);
    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);

    return 0;
}

// Public API

BusyTimer* busy_timer_alloc(void) {
    BusyTimer* instance = malloc(sizeof(BusyTimer));

    instance->thread = furi_thread_alloc_ex(TAG, 1024, busy_timer_thread, instance);
    instance->message_queue = furi_message_queue_alloc(1, sizeof(BusyTimerMessage));
    instance->config = busy_timer_config_default;

    furi_thread_start(instance->thread);
    return instance;
}

void busy_timer_free(BusyTimer* instance) {
    furi_assert(instance);

    furi_event_loop_stop(instance->event_loop);
    furi_thread_join(instance->thread);
    furi_thread_free(instance->thread);

    free(instance);
}

// Message handlers

static void busy_timer_start_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    FURI_LOG_I(TAG, "Starting");

    instance->state = BusyTimerStateIdle;
    busy_timer_next_state(instance, true);

    FURI_LOG_I(TAG, "Started");
}

static void busy_timer_stop_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    FURI_LOG_I(TAG, "Stopping");

    furi_event_loop_timer_stop(instance->timer);
    instance->state = BusyTimerStateIdle;

    FURI_LOG_I(TAG, "Stopped");
}

static void
    busy_timer_get_config_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    *data->config = instance->config;
}

static void
    busy_timer_set_config_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(instance);
    UNUSED(data);
}

static void busy_timer_get_state_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    data->state = instance->state;
}

static void
    busy_timer_set_callback_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    instance->callback = data->callback_info->callback;
    instance->callback_context = data->callback_info->context;
}

static const BusyTimerMessageHandler busy_timer_message_handlers[BusyTimerMessageTypeMax] = {
    [BusyTimerMessageTypeStart] = busy_timer_start_message_handler,
    [BusyTimerMessageTypeStop] = busy_timer_stop_message_handler,
    [BusyTimerMessageTypeGetConfig] = busy_timer_get_config_message_handler,
    [BusyTimerMessageTypeSetConfig] = busy_timer_set_config_message_handler,
    [BusyTimerMessageTypeGetState] = busy_timer_get_state_message_handler,
    [BusyTimerMessageTypeSetCallback] = busy_timer_set_callback_message_handler,
};
