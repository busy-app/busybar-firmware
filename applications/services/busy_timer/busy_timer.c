#include "busy_timer_i.h"

#include <furi_hal_rtc.h>

#include <busy/busy.h>
#include <loader/loader.h>
#include <desktop/desktop.h>

#ifdef BUSY_TIMER_TICK_DEBUG
#define TIME_MAX_LEN (14)
#endif

#define POLL_TIMER_PERIOD_MS    (S_TO_MS(1) / 30)
#define DEBOUNCE_TIMER_DELAY_MS (S_TO_MS(1))
// TODO: [FW-468] Add milliseconds support to RTC
#define TIMESTAMP_NOW_MS()      S_TO_MS((uint64_t)furi_hal_rtc_get_timestamp())

#define DEFAULT_CARD_ID "00000000-0000-0000-0000-000000000000"

typedef void (*const BusyTimerMessageHandler)(BusyTimer* instance, BusyTimerMessageData* data);

static const BusyTimerMessageHandler busy_timer_message_handlers[];

static const BusyTimerConfig busy_timer_config_default = {
    .mode = BusyTimerModeInterval,
    .time_mn = TIME_DEFAULT_MN,
    .work_time_mn = WORK_TIME_DEFAULT_MN,
    .rest_time_mn = REST_TIME_DEFAULT_MN,
    .cycle_count = CYCLE_COUNT_DEFAULT,
    .enable_intervals = ENABLE_INTERVALS_DEFAULT,
    .enable_autostart = ENABLE_AUTOSTART_DEFAULT,
    .enable_demo_mode = ENABLE_SPEED_DEFAULT,
};

static const char* busy_timer_mode_names[BusyTimerModeMax] = {
    [BusyTimerModeInfinite] = "Off",
    [BusyTimerModeSimple] = "Simple",
    [BusyTimerModeInterval] = "Interval",
};

static const char* busy_timer_state_names[BusyTimerStateMax] = {
    [BusyTimerStateIdle] = "Idle",
    [BusyTimerStateWork] = "Work",
    [BusyTimerStateRest] = "Rest",
};

static const char* busy_timer_get_state_name(BusyTimerState state) {
    furi_assert(state < BusyTimerStateMax);
    return busy_timer_state_names[state];
}

static const char* busy_timer_get_mode_name(BusyTimerMode mode) {
    furi_assert(mode < BusyTimerModeMax);
    return busy_timer_mode_names[mode];
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

static void busy_timer_start_app(void) {
    if(!furi_record_exists(RECORD_BUSY_APP)) {
        Desktop* desktop = furi_record_open(RECORD_DESKTOP);
        while(!desktop_replace_current_app(desktop, "busy", NULL)) {
            furi_thread_yield();
        }
        furi_record_close(RECORD_DESKTOP);
    }

    BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);
    busy_show_timer(busy_app);
    furi_record_close(RECORD_BUSY_APP);
}

static void busy_timer_exit_app(void) {
    if(furi_record_exists(RECORD_BUSY_APP)) {
        // Prevent the BUSY app from exiting
        furi_record_open(RECORD_BUSY_APP);
        // Request the app to stop
        Loader* loader = furi_record_open(RECORD_LOADER);
        furi_check(loader_stop(loader) == LoaderStatusOk);
        furi_record_close(RECORD_LOADER);
        // Now the app can exit
        furi_record_close(RECORD_BUSY_APP);
    }
}

static void busy_timer_notify_tick(const BusyTimer* instance) {
#ifdef BUSY_TIMER_TICK_DEBUG
    busy_timer_log_time(instance);
#endif

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeTick,
        .time = instance->time,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_mode_changed(const BusyTimer* instance) {
    FURI_LOG_D(TAG, "Mode changed: %s", busy_timer_get_mode_name(instance->mode));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeModeChanged,
        .mode = instance->mode,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_state_changed(const BusyTimer* instance) {
    FURI_LOG_D(TAG, "State changed: %s", busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeStateChanged,
        .state = instance->state,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_interval_ended(const BusyTimer* instance, bool force) {
    FURI_LOG_D(TAG, "Interval ended: %s", busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeIntervalEnded,
        .is_force_ended = force,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_paused(const BusyTimer* instance) {
    FURI_LOG_D(TAG, "Paused: %s", busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeTimerPaused,
        .timer_paused =
            {
                .is_paused = !instance->timer_running,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_user_interacted(BusyTimer* instance) {
    FURI_LOG_D(TAG, "User interacted: %s", busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeUserInteracted,
        .user_interacted =
            {
                .snapshot = instance->user_snapshot,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static BusyTimerState busy_timer_calc_state(const BusyTimer* instance) {
    BusyTimerState state;

    if(instance->state == BusyTimerStateIdle) {
        state = BusyTimerStateWork;

    } else if(instance->state == BusyTimerStateWork) {
        const bool interval_timer = instance->mode == BusyTimerModeInterval;
        const bool cycles_remaining = instance->cycles_done < instance->config.cycle_count;

        if(interval_timer && cycles_remaining) {
            state = BusyTimerStateRest;
        } else {
            state = BusyTimerStateIdle;
        }

    } else if(instance->state == BusyTimerStateRest) {
        state = BusyTimerStateWork;

    } else {
        furi_crash();
    }

    return state;
}

static uint32_t busy_timer_calc_remaining_time(const BusyTimer* instance) {
    uint32_t interval_s;

    if(instance->state == BusyTimerStateWork) {
        if(instance->mode == BusyTimerModeInterval) {
            interval_s = M_TO_S(instance->config.work_time_mn);
        } else {
            interval_s = M_TO_S(instance->config.time_mn);
        }

    } else if(instance->state == BusyTimerStateRest) {
        interval_s = M_TO_S(instance->config.rest_time_mn);
    } else {
        furi_crash();
    }

    return interval_s;
}

// Called BEFORE calculating the state
static uint32_t busy_timer_calc_cycles_done(const BusyTimer* instance) {
    if((instance->state == BusyTimerStateIdle) || (instance->mode != BusyTimerModeInterval)) {
        return 0;
    } else if(instance->state == BusyTimerStateWork) {
        return instance->cycles_done + 1;
    } else {
        return instance->cycles_done;
    }
}

static uint32_t busy_timer_calc_increment(const BusyTimer* instance) {
    if(instance->config.enable_demo_mode) {
        if(instance->time.remain_s > 60) {
            return 60;
        } else if(instance->time.remain_s > 30) {
            return 30;
        } else if(instance->time.remain_s > 15) {
            return 15;
        } else if(instance->time.remain_s > 5) {
            return 5;
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

static bool busy_timer_is_running(const BusyTimer* instance) {
    return instance->timer_running;
}

static void busy_timer_start_timer(BusyTimer* instance) {
    if(instance->mode != BusyTimerModeInfinite) {
        furi_event_loop_timer_start(instance->poll_timer, POLL_TIMER_PERIOD_MS);
    }

    instance->prev_tick_timestamp_ms = TIMESTAMP_NOW_MS();
    instance->timer_running = true;
}

static void busy_timer_stop_timer(BusyTimer* instance) {
    furi_event_loop_timer_stop(instance->poll_timer);
    instance->timer_running = false;
}

static void busy_timer_infinite_to_simple(BusyTimer* instance) {
    instance->mode = BusyTimerModeSimple;
    instance->time.remain_s = M_TO_S(BUSY_TIMER_TIME_INCREMENT_MN);
    instance->time.elapsed_s = 0;

    busy_timer_start_timer(instance);
    busy_timer_notify_mode_changed(instance);
    busy_timer_notify_state_changed(instance);
    busy_timer_notify_tick(instance);
}

static void busy_timer_next_state(BusyTimer* instance, bool force) {
    FURI_LOG_I(TAG, "Current state: %s", busy_timer_get_state_name(instance->state));

    instance->cycles_done = busy_timer_calc_cycles_done(instance);
    instance->state = busy_timer_calc_state(instance);

    if(instance->state != BusyTimerStateIdle) {
        instance->time.elapsed_s = 0;
        instance->time.remain_s = busy_timer_calc_remaining_time(instance);

        if(instance->config.enable_autostart || force) {
            busy_timer_start_timer(instance);
            busy_timer_notify_state_changed(instance);
            busy_timer_notify_tick(instance);

        } else {
            busy_timer_stop_timer(instance);
            busy_timer_notify_interval_ended(instance, force);
        }

    } else {
        busy_timer_stop_timer(instance);
        busy_timer_notify_interval_ended(instance, force);
    }
}

static void busy_timer_update(BusyTimer* instance, uint64_t timestamp_ms) {
    do {
        // Got snapshot from a peer with a clock that is ahead of ours
        if(timestamp_ms < instance->prev_tick_timestamp_ms) {
            instance->prev_tick_timestamp_ms = timestamp_ms;
            busy_timer_notify_tick(instance);
            break;
        }

        const uint32_t dt_s = MS_TO_S(timestamp_ms - instance->prev_tick_timestamp_ms);

        // Too early for a tick
        if(dt_s == 0) {
            break;
        }

        for(uint32_t i = 0; i < dt_s; ++i) {
            const uint32_t inc_s = busy_timer_calc_increment(instance);
            const bool is_last = (i == (dt_s - 1));

            if(instance->time.remain_s >= inc_s) {
                instance->time.remain_s -= inc_s;
                instance->time.elapsed_s += inc_s;

                if(is_last) {
                    busy_timer_notify_tick(instance);
                }

            } else {
                // Force every but last transition
                busy_timer_next_state(instance, !is_last);
                // Break out of the loop if already finished
                if(instance->state == BusyTimerStateIdle) {
                    break;
                }
            }
        }

        instance->prev_tick_timestamp_ms = timestamp_ms;
    } while(false);
}

static uint32_t busy_timer_get_interval_index(const BusyTimer* instance) {
    return 2 * instance->cycles_done + (instance->state == BusyTimerStateRest ? 1 : 0);
}

static void busy_timer_make_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    snapshot->timestamp_ms = TIMESTAMP_NOW_MS();

    if(instance->state != BusyTimerStateIdle) {
        BusyTimerSnapshotCommon* common = &snapshot->common;
        strcpy(common->card_id, DEFAULT_CARD_ID);
        common->is_paused = !busy_timer_is_running(instance);

        const BusyTimerMode mode = instance->mode;

        if(mode == BusyTimerModeInfinite) {
            snapshot->type = BusyTimerSnapshotTypeInfinite;

        } else if(mode == BusyTimerModeSimple) {
            snapshot->type = BusyTimerSnapshotTypeSimple;

            const BusyTimerTime* time = &instance->time;

            BusyTimerSnapshotSimple* simple = &snapshot->simple;
            simple->time_left_ms = S_TO_MS(time->remain_s);

        } else if(mode == BusyTimerModeInterval) {
            snapshot->type = BusyTimerSnapshotTypeInterval;

            const BusyTimerTime* time = &instance->time;
            const BusyTimerConfig* config = &instance->config;

            BusyTimerSnapshotInterval* interval = &snapshot->interval;
            BusyTimerIntervalState* state = &interval->state;
            state->index = busy_timer_get_interval_index(instance);
            state->time_left_ms = S_TO_MS(time->remain_s);
            state->time_total_ms = S_TO_MS(time->elapsed_s + time->remain_s);

            BusyTimerIntervalSettings* settings = &interval->settings;
            settings->work_time_ms = M_TO_MS(config->work_time_mn);
            settings->rest_time_ms = M_TO_MS(config->rest_time_mn);
            settings->cycles_count = config->cycle_count;
            settings->is_autostart_enabled = config->enable_autostart;

        } else {
            furi_crash("Invalid BusyTimerMode value");
        }

    } else {
        snapshot->type = BusyTimerSnapshotTypeNotStarted;
    }
}

static void busy_timer_apply_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    const uint64_t snapshot_timestamp_ms = snapshot->timestamp_ms;

    if(snapshot_timestamp_ms <= instance->user_snapshot.timestamp_ms) {
        // Ignore snapshots that are older than the last known one
        FURI_LOG_D(TAG, "Ignoring stale/own snapshot with timestamp %llu", snapshot_timestamp_ms);
        return;
    }

    busy_timer_stop_timer(instance);

    const BusyTimerSnapshotType type = snapshot->type;

    if(type == BusyTimerSnapshotTypeNotStarted) {
        instance->state = BusyTimerStateIdle;
        busy_timer_exit_app();
        return;
    }

    BusyTimerMode new_mode;
    BusyTimerState new_state;

    if(type == BusyTimerSnapshotTypeInfinite) {
        new_mode = BusyTimerModeInfinite;
        new_state = BusyTimerStateWork;

    } else if(type == BusyTimerSnapshotTypeSimple) {
        const BusyTimerSnapshotSimple* simple = &snapshot->simple;

        new_mode = BusyTimerModeSimple;
        new_state = BusyTimerStateWork;

        instance->time.elapsed_s = 0;
        instance->time.remain_s = MS_TO_S(simple->time_left_ms);

    } else if(type == BusyTimerSnapshotTypeInterval) {
        const BusyTimerSnapshotInterval* interval = &snapshot->interval;
        const BusyTimerIntervalState* interval_state = &interval->state;
        const BusyTimerIntervalSettings* interval_settings = &interval->settings;

        new_mode = BusyTimerModeInterval;
        new_state = interval_state->index % 2 ? BusyTimerStateRest : BusyTimerStateWork;

        instance->cycles_done = interval_state->index / 2;
        instance->time.elapsed_s =
            MS_TO_S(interval_state->time_total_ms - interval_state->time_left_ms);
        instance->time.remain_s = MS_TO_S(interval_state->time_left_ms);

        instance->config.cycle_count = interval_settings->cycles_count;
        instance->config.work_time_mn = MS_TO_M(interval_settings->work_time_ms);
        instance->config.rest_time_mn = MS_TO_M(interval_settings->rest_time_ms);
        instance->config.enable_autostart = interval_settings->is_autostart_enabled;
        instance->config.enable_intervals = true;

    } else {
        furi_crash("Invalid BusyTimerSnapshotType value");
    }

    instance->mode = new_mode;
    instance->state = new_state;
    instance->config.mode = new_mode;

    if(!snapshot->common.is_paused) {
        busy_timer_start_timer(instance);
    }

    instance->prev_tick_timestamp_ms = snapshot_timestamp_ms;

    busy_timer_start_app();

    busy_timer_notify_mode_changed(instance);
    busy_timer_notify_state_changed(instance);
    busy_timer_notify_paused(instance);
}

static void busy_timer_schedule_notify_user_interacted(BusyTimer* instance) {
    busy_timer_make_snapshot(instance, &instance->user_snapshot);
    furi_event_loop_timer_start(instance->debounce_timer, DEBOUNCE_TIMER_DELAY_MS);
}

static void busy_timer_poll_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;
    busy_timer_update(instance, TIMESTAMP_NOW_MS());
}

static void busy_timer_debounce_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;
    busy_timer_notify_user_interacted(instance);
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

// Public API

const char** busy_timer_get_mode_names(void) {
    return busy_timer_mode_names;
}

FuriPubSub* busy_timer_get_pubsub(const BusyTimer* instance) {
    furi_check(instance);
    return instance->event_pubsub;
}

// Message handlers

static void busy_timer_start_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    FURI_LOG_I(TAG, "Starting");

    instance->mode = instance->config.mode;
    busy_timer_notify_mode_changed(instance);

    if(instance->state == BusyTimerStateIdle) {
        busy_timer_next_state(instance, true);

        FURI_LOG_I(TAG, "Started");

    } else {
        busy_timer_start_timer(instance);
        busy_timer_notify_state_changed(instance);
        busy_timer_notify_tick(instance);

        FURI_LOG_I(TAG, "Resumed");
    }

    busy_timer_schedule_notify_user_interacted(instance);
}

static void busy_timer_stop_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    busy_timer_stop_timer(instance);
    instance->state = BusyTimerStateIdle;

    FURI_LOG_I(TAG, "Stopped");

    busy_timer_schedule_notify_user_interacted(instance);
}

static void
    busy_timer_get_config_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    *data->config = instance->config;
}

static void
    busy_timer_set_config_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    instance->config = *data->config_c;

    const BusySettings settings = {
        .timer_config = instance->config,
    };

    busy_settings_save(&settings);
}

static void busy_timer_get_state_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    *data->state = instance->state;
}

static void busy_timer_get_time_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    *data->time = instance->time;
}

static void
    busy_timer_get_cycles_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    data->cycles->total_count = instance->config.cycle_count;
    data->cycles->done_count = instance->cycles_done;
}

static void busy_timer_add_time_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    if(!busy_timer_is_running(instance)) {
        // Ignore if the timer is not running (paused)
        return;
    }

    if(instance->mode == BusyTimerModeInfinite) {
        if(data->add_time_mn > 0) {
            // Special case: start a Simple timer
            busy_timer_infinite_to_simple(instance);
        }
        return;
    }

    int32_t time_remaining_s = instance->time.remain_s;
    int32_t increment_s = M_TO_S(data->add_time_mn);

    // Ignore if the remaining time is below minimum
    if((increment_s < 0) && (time_remaining_s < BUSY_TIMER_TIME_MIN_S)) {
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

    busy_timer_start_timer(instance);
    busy_timer_notify_tick(instance);
    busy_timer_schedule_notify_user_interacted(instance);

    FURI_LOG_I(TAG, "Interval override");
}

static void busy_timer_toggle_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    if(busy_timer_is_running(instance)) {
        busy_timer_stop_timer(instance);
        FURI_LOG_I(TAG, "Paused");

    } else {
        busy_timer_start_timer(instance);
        FURI_LOG_I(TAG, "Resumed");
    }

    busy_timer_notify_paused(instance);
    busy_timer_schedule_notify_user_interacted(instance);
}

static void busy_timer_skip_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    if(busy_timer_is_running(instance)) {
        busy_timer_next_state(instance, true);
        busy_timer_schedule_notify_user_interacted(instance);
    }
}

static void
    busy_timer_get_snapshot_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    busy_timer_make_snapshot(instance, data->snapshot);
}

static void
    busy_timer_set_snapshot_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    busy_timer_apply_snapshot(instance, data->snapshot_c);
}

// Service

static BusyTimer* busy_timer_alloc(void) {
    BusyTimer* instance = malloc(sizeof(BusyTimer));

    instance->event_loop = furi_event_loop_alloc();
    instance->poll_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_timer_poll_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->debounce_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_timer_debounce_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->message_queue = furi_message_queue_alloc(1, sizeof(BusyTimerMessage));
    instance->event_pubsub = furi_pubsub_alloc();

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        busy_timer_message_queue_callback,
        instance);

    BusySettings settings;
    if(!busy_settings_load(&settings)) {
        FURI_LOG_W(TAG, "Loading default settings");
        settings.timer_config = busy_timer_config_default;
        busy_settings_save(&settings);
    }

    instance->config = settings.timer_config;

    furi_record_create(RECORD_BUSY_TIMER, instance);

    return instance;
}

int busy_timer_srv(void* arg) {
    UNUSED(arg);

    BusyTimer* instance = busy_timer_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const BusyTimerMessageHandler busy_timer_message_handlers[BusyTimerMessageTypeMax] = {
    [BusyTimerMessageTypeStart] = busy_timer_start_message_handler,
    [BusyTimerMessageTypeStop] = busy_timer_stop_message_handler,
    [BusyTimerMessageTypeGetConfig] = busy_timer_get_config_message_handler,
    [BusyTimerMessageTypeSetConfig] = busy_timer_set_config_message_handler,
    [BusyTimerMessageTypeGetState] = busy_timer_get_state_message_handler,
    [BusyTimerMessageTypeGetTime] = busy_timer_get_time_message_handler,
    [BusyTimerMessageTypeGetCycles] = busy_timer_get_cycles_message_handler,
    [BusyTimerMessageTypeAddTime] = busy_timer_add_time_message_handler,
    [BusyTimerMessageTypeToggle] = busy_timer_toggle_message_handler,
    [BusyTimerMessageTypeSkip] = busy_timer_skip_message_handler,
    [BusyTimerMessageTypeGetSnapshot] = busy_timer_get_snapshot_message_handler,
    [BusyTimerMessageTypeSetSnapshot] = busy_timer_set_snapshot_message_handler,
};
