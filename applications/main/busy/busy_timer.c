#include "busy_timer_i.h"

#ifdef BUSY_TIMER_TICK_DEBUG
#define TIME_MAX_LEN (14)
#endif

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

static const char* busy_timer_get_state_name(BusyTimerState state) {
    furi_assert(state < BusyTimerStateMax);

    static const char* state_names[BusyTimerStateMax] = {
        [BusyTimerStateIdle] = "Idle",
        [BusyTimerStateWork] = "Work",
        [BusyTimerStateRest] = "Rest",
    };

    return state_names[state];
}

#ifdef BUSY_TIMER_TICK_DEBUG
static void busy_timer_get_time_str(uint32_t time_s, char buf[TIME_MAX_LEN]) {
    const uint32_t h = S_TO_H(time_s);
    const uint32_t m = S_TO_M(time_s - H_TO_S(h));
    const uint32_t s = time_s - H_TO_S(h) - M_TO_S(m);

    if(h) {
        snprintf(buf, TIME_MAX_LEN, "%2lu:%02lu:%02lu", h, m, s);
    } else {
        snprintf(buf, TIME_MAX_LEN, "%02lu:%02lu", m, s);
    }
}

static void busy_timer_log_time(BusyTimer* instance) {
    char buf[TIME_MAX_LEN];

    busy_timer_get_time_str(instance->time.elapsed_s, buf);
    FURI_LOG_D(TAG, "Elapsed: %s", buf);

    busy_timer_get_time_str(instance->time.remain_s, buf);
    FURI_LOG_D(TAG, "Remaining: %s", buf);
}
#endif

static void busy_timer_notify_tick(BusyTimer* instance) {
#ifdef BUSY_TIMER_TICK_DEBUG
    busy_timer_log_time(instance);
#endif

    if(instance->callback) {
        const BusyTimerEvent event = {
            .type = BusyTimerEventTypeTick,
            .time = instance->time,
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

static uint32_t busy_timer_calc_remaining_time(BusyTimer* instance) {
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

// static void busy_play_finished_sound(BusyApp* instance) {
//     if(instance->enable_sound) {
//         if(instance->state == BusyTimerStateWork) {
//             audio_play_file(instance->audio, EXT_PATH("audio/work_finished.snd"));
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             audio_play_file(instance->audio, EXT_PATH("audio/rest_finished.snd"));
//         }
//     }
// }

// static void busy_play_countdown_sound(BusyApp* instance) {
//     if(instance->enable_sound && instance->interval_time_left_s <= 4) {
//         if(instance->state == BusyTimerStateWork) {
//             audio_play_file(instance->audio, EXT_PATH("audio/work_countdown.snd"));
//         } else if(instance->state == BusyTimerStateRest || instance->state == BusyTimerStateLongRest) {
//             audio_play_file(instance->audio, EXT_PATH("audio/rest_countdown.snd"));
//         }
//     }
// }

void busy_timer_next_state(BusyTimer* instance) {
    FURI_LOG_I(TAG, "Current state: %s", busy_timer_get_state_name(instance->state));

    instance->state = busy_timer_calc_state(instance);
    instance->time.elapsed_s = 0;
    instance->time.remain_s = busy_timer_calc_remaining_time(instance);

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

    if(instance->time.remain_s) {
        instance->time.remain_s--;
        instance->time.elapsed_s++;

        busy_timer_notify_tick(instance);

    } else {
        busy_timer_next_state(instance);
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
    busy_timer_next_state(instance);

    FURI_LOG_I(TAG, "Started");
}

static void busy_timer_stop_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

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
    instance->config = *data->config_c;
}

static void busy_timer_get_state_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    data->state = instance->state;
}

static void
    busy_timer_set_callback_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    instance->callback = data->callback_info->callback;
    instance->callback_context = data->callback_info->context;
}

static void busy_timer_add_time_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    int32_t time_remaining_s = instance->time.remain_s;
    int32_t increment_s = M_TO_S(data->add_time_mn);

    if((increment_s < 0) && (time_remaining_s < BUSY_TIMER_TIME_MIN_S)) {
        // Cannot decrease interval below minimum time
        return;
    }

    /* Round to the nearest increment multiple with
     * respect to the direction (sign), e.g:
     *
     * 15:00 + 5:00 = 20:00
     * 17:32 + 5:00 = 20:00
     * 15:00 - 5:00 = 10:00
     * 17:32 - 5:00 = 15:00
     *
     */

    const int32_t remainder_s = time_remaining_s % increment_s;

    if(remainder_s) {
        if(increment_s > 0) {
            increment_s -= remainder_s;
        } else {
            increment_s = -remainder_s;
        }
    }

    time_remaining_s += increment_s;

    instance->time.remain_s =
        CLAMP(time_remaining_s, BUSY_TIMER_TIME_MAX_S, BUSY_TIMER_TIME_MIN_S);

    furi_event_loop_timer_restart(instance->timer);
    busy_timer_notify_tick(instance);

    FURI_LOG_I(TAG, "Interval override");
}

static void busy_timer_toggle_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    if(furi_event_loop_timer_is_running(instance->timer)) {
        furi_event_loop_timer_stop(instance->timer);
        FURI_LOG_I(TAG, "Paused");
    } else {
        furi_event_loop_timer_restart(instance->timer);
        FURI_LOG_I(TAG, "Resumed");
    }
}

static void busy_timer_skip_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);
    busy_timer_next_state(instance);
}

static const BusyTimerMessageHandler busy_timer_message_handlers[BusyTimerMessageTypeMax] = {
    [BusyTimerMessageTypeStart] = busy_timer_start_message_handler,
    [BusyTimerMessageTypeStop] = busy_timer_stop_message_handler,
    [BusyTimerMessageTypeGetConfig] = busy_timer_get_config_message_handler,
    [BusyTimerMessageTypeSetConfig] = busy_timer_set_config_message_handler,
    [BusyTimerMessageTypeGetState] = busy_timer_get_state_message_handler,
    [BusyTimerMessageTypeSetCallback] = busy_timer_set_callback_message_handler,
    [BusyTimerMessageTypeAddTime] = busy_timer_add_time_message_handler,
    [BusyTimerMessageTypeToggle] = busy_timer_toggle_message_handler,
    [BusyTimerMessageTypeSkip] = busy_timer_skip_message_handler,
};
