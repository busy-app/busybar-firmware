#include "busy_timer_i.h"

#include <furi_hal_rtc.h>

#ifdef BUSY_TIMER_TICK_DEBUG
#define TIME_MAX_LEN (14)
#endif

#define API_QUEUE_SIZE (4)

#define POLL_TIMER_PERIOD_MS    (S_TO_MS(1) / 30)
#define DEBOUNCE_TIMER_DELAY_MS (250)

#define TIMER_MQTT_PREFIX               "busy"
#define TIMER_SNAPSHOT_MQTT_TOPIC       TIMER_MQTT_PREFIX "/snapshot"
#define TIMER_PROFILE_MQTT_PREFIX       TIMER_MQTT_PREFIX "/profiles"
#define TIMER_PROFILE_BUSY_MQTT_TOPIC   TIMER_PROFILE_MQTT_PREFIX "/busy"
#define TIMER_PROFILE_CUSTOM_MQTT_TOPIC TIMER_PROFILE_MQTT_PREFIX "/custom"

#define TIMER_SNAPSHOT_MQTT_QOS MqttQosAtLeastOnce
#define TIMER_PROFILE_MQTT_QOS  MqttQosAtLeastOnce

typedef void (*const BusyTimerApiMessageHandler)(
    BusyTimer* instance,
    BusyTimerApiMessageData* data);

static const BusyTimerApiMessageHandler busy_timer_api_message_handlers[];

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

static const char* busy_timer_profile_names[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = "busy",
    [BusyTimerProfileIdCustom] = "custom",
};

static const char* busy_timer_mqtt_topics[BusyTimerProfileIdMax] = {
    [BusyTimerProfileIdBusy] = TIMER_PROFILE_BUSY_MQTT_TOPIC,
    [BusyTimerProfileIdCustom] = TIMER_PROFILE_CUSTOM_MQTT_TOPIC,
};

static const char* busy_timer_get_state_name(BusyTimerState state) {
    furi_assert(state < BusyTimerStateMax);
    return busy_timer_state_names[state];
}

static const char* busy_timer_get_mode_name(BusyTimerMode mode) {
    furi_assert(mode < BusyTimerModeMax);
    return busy_timer_mode_names[mode];
}

const char* busy_timer_get_profile_name(BusyTimerProfileId profile_id) {
    furi_assert(profile_id < BusyTimerProfileIdMax);
    return busy_timer_profile_names[profile_id];
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

static void busy_timer_notify_tick(const BusyTimer* instance) {
#ifdef BUSY_TIMER_TICK_DEBUG
    busy_timer_log_time(instance);
#endif

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeTick,
        .tick =
            {
                .time_elapsed_s = instance->time_elapsed_s,
                .time_remaining_s = instance->time_remaining_s,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_mode_changed(const BusyTimer* instance) {
    const BusyTimerMode timer_mode = instance->timer_config.mode;
    FURI_LOG_D(TAG, "New mode: %s", busy_timer_get_mode_name(timer_mode));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeModeChanged,
        .mode_changed.mode = timer_mode,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_state_changed(const BusyTimer* instance) {
    FURI_LOG_D(TAG, "New state: %s", busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeStateChanged,
        .state_changed.state = instance->state,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_interval_ended(const BusyTimer* instance, bool force) {
    FURI_LOG_D(TAG, "Interval ended: %s", busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeIntervalEnded,
        .interval_ended.is_forced = force,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_paused(const BusyTimer* instance) {
    const bool is_paused = !busy_timer_is_running(instance);

    const char* message = is_paused ? "Paused" : "Resumed";
    FURI_LOG_D(TAG, "%s: %s", message, busy_timer_get_state_name(instance->state));

    BusyTimerEvent event = {
        .type = BusyTimerEventTypePaused,
        .paused.is_paused = is_paused,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void
    busy_timer_notify_profile_changed(const BusyTimer* instance, BusyTimerProfileId profile_id) {
    const BusyTimerProfile* profile = &instance->settings[profile_id].profile;

    FURI_LOG_D(
        TAG,
        "Profile changed: \"%s\"\r\n"
        "\ttitle:     %s\r\n"
        "\tcard id:   %s\r\n"
        "\ttimestamp: %llu",
        busy_timer_get_profile_name(profile_id),
        profile->metadata.title,
        profile->metadata.card_id,
        profile->timestamp_ms);

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeProfileChanged,
        .profile_changed =
            {
                .profile_id = profile_id,
                .profile = *profile,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_session_started(const BusyTimer* instance) {
    FURI_LOG_I(TAG, "Session started");

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeSessionStarted,
        .session_started =
            {
                .source = instance->session_source,
                .profile_id = instance->active_profile_id,
                .app_config = instance->app_config,
                .timer_config = instance->timer_config,
                .is_demo_mode_enabled = instance->is_demo_mode_enabled,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void
    busy_timer_notify_session_ended(const BusyTimer* instance, BusyTimerSessionOutcome outcome) {
    FURI_LOG_I(
        TAG,
        "Session ended: %s",
        outcome == BusyTimerSessionOutcomeCompleted ? "completed" : "stopped");

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeSessionEnded,
        .session_ended =
            {
                .outcome = outcome,
                .source = instance->session_source,
                .time_elapsed_s = instance->time_elapsed_s,
                .current_interval_index = instance->current_interval_index,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_snapshot_created(const BusyTimer* instance) {
    const BusyTimerSnapshot* snapshot = &instance->last_known_snapshot;

    FURI_LOG_D(TAG, "Snapshot created with timestamp: %llu", snapshot->timestamp_ms);

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeSnapshotCreated,
        .snapshot_created =
            {
                .snapshot = *snapshot,
            },
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void busy_timer_notify_initial_state(const BusyTimer* instance) {
    busy_timer_notify_mode_changed(instance);
    busy_timer_notify_tick(instance);
    busy_timer_notify_state_changed(instance);
    busy_timer_notify_paused(instance);
}

static BusyTimerState busy_timer_calc_next_state(const BusyTimer* instance) {
    BusyTimerState next_state;

    const BusyTimerConfig* config = &instance->timer_config;
    const BusyTimerMode timer_mode = config->mode;

    if(instance->state == BusyTimerStateIdle) {
        next_state = BusyTimerStateWork;

    } else if(instance->state == BusyTimerStateWork) {
        if(timer_mode == BusyTimerModeInterval) {
            const uint32_t cycles_count = config->interval.cycles_count;

            if(instance->current_interval_index < cycles_count * 2 - 1) {
                next_state = BusyTimerStateRest;
            } else {
                next_state = BusyTimerStateIdle;
            }

        } else {
            next_state = BusyTimerStateIdle;
        }

    } else if(instance->state == BusyTimerStateRest) {
        if(timer_mode == BusyTimerModeInterval) {
            next_state = BusyTimerStateWork;
        } else {
            furi_crash("Invalid timer mode in rest state");
        }

    } else {
        furi_crash("Invalid BusyTimerState value");
    }

    return next_state;
}

static uint32_t busy_timer_calc_remaining_time(const BusyTimer* instance) {
    uint32_t interval_s;

    const BusyTimerConfig* config = &instance->timer_config;
    const BusyTimerMode timer_mode = config->mode;

    if(timer_mode == BusyTimerModeInfinite) {
        interval_s = UINT32_MAX;

    } else if(timer_mode == BusyTimerModeSimple) {
        const BusyTimerSimpleConfig* simple = &config->simple;

        if(instance->state == BusyTimerStateWork) {
            interval_s = MS_TO_S(simple->total_time_ms);
        } else {
            furi_crash("Invalid timer state in simple mode");
        }

    } else if(timer_mode == BusyTimerModeInterval) {
        const BusyTimerIntervalConfig* interval = &config->interval;

        if(instance->state == BusyTimerStateWork) {
            interval_s = MS_TO_S(interval->work_time_ms);
        } else if(instance->state == BusyTimerStateRest) {
            interval_s = MS_TO_S(interval->rest_time_ms);
        } else {
            furi_crash("Invalid timer state in interval mode");
        }

    } else {
        furi_crash("Invalid BusyTimerMode value");
    }

    return interval_s;
}

// Called BEFORE calculating the state
static uint32_t busy_timer_calc_interval_index(const BusyTimer* instance) {
    uint32_t interval_index = 0;

    if(instance->state != BusyTimerStateIdle) {
        if(instance->timer_config.mode == BusyTimerModeInterval) {
            interval_index = instance->current_interval_index + 1;
        }
    }

    return interval_index;
}

static uint32_t busy_timer_calc_increment(const BusyTimer* instance) {
    if(instance->is_demo_mode_enabled) {
        if(instance->time_remaining_s > 60) {
            return 60;
        } else if(instance->time_remaining_s > 30) {
            return 30;
        } else if(instance->time_remaining_s > 15) {
            return 15;
        } else if(instance->time_remaining_s > 5) {
            return 5;
        } else {
            return 1;
        }
    } else {
        return 1;
    }

    return 1;
}

bool busy_timer_is_running(const BusyTimer* instance) {
    return instance->is_timer_running;
}

static void busy_timer_start_timer(BusyTimer* instance) {
    if(instance->timer_config.mode != BusyTimerModeInfinite) {
        furi_event_loop_timer_start(instance->poll_timer, POLL_TIMER_PERIOD_MS);
    }

    instance->prev_tick_timestamp_ms = furi_hal_rtc_get_timestamp_ms();
    instance->is_timer_running = true;
}

static void busy_timer_stop_timer(BusyTimer* instance) {
    furi_event_loop_timer_stop(instance->poll_timer);
    instance->is_timer_running = false;
}

static void busy_timer_infinite_to_simple(BusyTimer* instance) {
    instance->timer_config.mode = BusyTimerModeSimple;
    instance->time_remaining_s = M_TO_S(BUSY_TIMER_TIME_INCREMENT_MN);
    instance->time_elapsed_s = 0;

    busy_timer_start_timer(instance);
    busy_timer_notify_initial_state(instance);
}

static void
    busy_timer_next_state(BusyTimer* instance, bool is_forced, BusyTimerSessionOutcome outcome) {
    const char* old_state_name = busy_timer_get_state_name(instance->state);

    instance->current_interval_index = busy_timer_calc_interval_index(instance);
    instance->state = busy_timer_calc_next_state(instance);

    FURI_LOG_I(
        TAG, "State change: %s -> %s", old_state_name, busy_timer_get_state_name(instance->state));

    if(instance->state != BusyTimerStateIdle) {
        instance->time_elapsed_s = 0;
        instance->time_remaining_s = busy_timer_calc_remaining_time(instance);

        bool is_autostart_enabled = false;

        if(instance->timer_config.mode == BusyTimerModeInterval) {
            is_autostart_enabled = instance->timer_config.interval.is_autostart_enabled;
        }

        if(is_autostart_enabled || is_forced) {
            busy_timer_start_timer(instance);
            busy_timer_notify_state_changed(instance);
            busy_timer_notify_tick(instance);

        } else {
            busy_timer_stop_timer(instance);
            busy_timer_notify_interval_ended(instance, is_forced);
        }

    } else {
        busy_timer_stop_timer(instance);
        busy_timer_notify_state_changed(instance);
        busy_timer_notify_interval_ended(instance, is_forced);
        busy_timer_notify_session_ended(instance, outcome);
    }
}

static void busy_timer_update(BusyTimer* instance, time_t timestamp_ms) {
    do {
        // Got snapshot from a peer with a clock that is ahead of ours
        if(timestamp_ms < instance->prev_tick_timestamp_ms) {
            instance->prev_tick_timestamp_ms = timestamp_ms;
            busy_timer_notify_tick(instance);
            break;
        }

        const uint32_t dt_ms = timestamp_ms - instance->prev_tick_timestamp_ms;
        const uint32_t dt_s = MS_TO_S(dt_ms);

        // Too early for a tick
        if(dt_s == 0) {
            break;
        }

        for(uint32_t i = 0; i < dt_s; ++i) {
            const uint32_t inc_s = busy_timer_calc_increment(instance);
            const bool is_last = (i == (dt_s - 1));

            if(instance->time_remaining_s >= inc_s) {
                instance->time_remaining_s -= inc_s;
                instance->time_elapsed_s += inc_s;

                if(is_last) {
                    busy_timer_notify_tick(instance);
                }

            } else {
                busy_timer_next_state(instance, false, BusyTimerSessionOutcomeCompleted);

                if(instance->state == BusyTimerStateIdle) {
                    break;
                }

                const BusyTimerConfig* config = &instance->timer_config;

                const bool is_interval_timer = config->mode == BusyTimerModeInterval;
                const bool is_autostart_enabled = config->interval.is_autostart_enabled;

                if(is_interval_timer && !is_autostart_enabled) {
                    break;
                }
            }
        }

        const uint32_t remainder_ms = dt_ms - S_TO_MS(dt_s);
        instance->prev_tick_timestamp_ms = timestamp_ms - remainder_ms;

    } while(false);
}

static void busy_timer_fill_snapshot_common(BusyTimer* instance, BusyTimerSnapshotCommon* common) {
    strcpy(common->card_id, instance->card_id);
    common->is_paused = !busy_timer_is_running(instance);
}

static void busy_timer_capture_snapshot(BusyTimer* instance) {
    BusyTimerSnapshot* snapshot = &instance->last_known_snapshot;

    snapshot->timestamp_ms = furi_hal_rtc_get_timestamp_ms();

    if(instance->state != BusyTimerStateIdle) {
        const BusyTimerMode timer_mode = instance->timer_config.mode;

        if(timer_mode == BusyTimerModeInfinite) {
            snapshot->type = BusyTimerSnapshotTypeInfinite;

            BusyTimerSnapshotInfinite* infinite = &snapshot->infinite;
            busy_timer_fill_snapshot_common(instance, &infinite->common);

        } else if(timer_mode == BusyTimerModeSimple) {
            snapshot->type = BusyTimerSnapshotTypeSimple;

            BusyTimerSnapshotSimple* simple = &snapshot->simple;
            busy_timer_fill_snapshot_common(instance, &simple->common);

            simple->time_left_ms = S_TO_MS(instance->time_remaining_s);

        } else if(timer_mode == BusyTimerModeInterval) {
            snapshot->type = BusyTimerSnapshotTypeInterval;

            BusyTimerSnapshotInterval* interval = &snapshot->interval;
            busy_timer_fill_snapshot_common(instance, &interval->common);

            BusyTimerIntervalState* state = &interval->state;
            state->index = instance->current_interval_index;
            state->time_left_ms = S_TO_MS(instance->time_remaining_s);
            state->time_total_ms = S_TO_MS(instance->time_elapsed_s + instance->time_remaining_s);

            interval->config = instance->timer_config.interval;

        } else {
            furi_crash("Invalid BusyTimerMode value");
        }

    } else {
        snapshot->type = BusyTimerSnapshotTypeNotStarted;
    }

    snapshot->app_config = instance->app_config;
}

static void busy_timer_store_saved_state(BusyTimer* instance) {
    BusyTimerSavedState* saved_state = &instance->saved_state;
    const BusyTimerSnapshot* last_known_snapshot = &instance->last_known_snapshot;

    if(last_known_snapshot->timestamp_ms > saved_state->snapshot.timestamp_ms) {
        saved_state->snapshot = *last_known_snapshot;
        busy_timer_saved_state_save(saved_state);
    }
}

static void busy_timer_publish_last_known_snapshot(const BusyTimer* instance) {
    busy_timer_notify_snapshot_created(instance);

    const BusyTimerSnapshot* snapshot = &instance->last_known_snapshot;
    char* snapshot_str = busy_timer_snapshot_serialize(snapshot);
    furi_check(snapshot_str);

    mqtt_publish(
        instance->mqtt,
        TIMER_SNAPSHOT_MQTT_QOS,
        TIMER_SNAPSHOT_MQTT_TOPIC,
        snapshot_str,
        strlen(snapshot_str));

    free(snapshot_str);
}

static void busy_timer_publish_profile(BusyTimer* instance, BusyTimerProfileId profile_id) {
    busy_timer_notify_profile_changed(instance, profile_id);

    const BusyTimerProfile* profile = &instance->settings[profile_id].profile;
    char* profile_str = busy_timer_profile_serialize(profile);
    furi_check(profile_str);

    mqtt_publish(
        instance->mqtt,
        TIMER_PROFILE_MQTT_QOS,
        busy_timer_mqtt_topics[profile_id],
        profile_str,
        strlen(profile_str));

    free(profile_str);
}

static void busy_timer_schedule_publish_last_known_snapshot(BusyTimer* instance) {
    if(instance->snapshot_update_count == 0) {
        busy_timer_publish_last_known_snapshot(instance);
        busy_timer_store_saved_state(instance);
    }

    ++instance->snapshot_update_count;
    furi_event_loop_timer_start(instance->snapshot_timer, DEBOUNCE_TIMER_DELAY_MS);
}

static void
    busy_timer_schedule_publish_profile(BusyTimer* instance, BusyTimerProfileId profile_id) {
    if(instance->profile_update_count[profile_id] == 0) {
        busy_timer_publish_profile(instance, profile_id);
    }

    ++instance->profile_update_count[profile_id];
    furi_event_loop_timer_start(instance->profile_timer, DEBOUNCE_TIMER_DELAY_MS);
}

static void busy_timer_capture_and_publish_snapshot(BusyTimer* instance) {
    busy_timer_capture_snapshot(instance);
    busy_timer_schedule_publish_last_known_snapshot(instance);
}

static void busy_timer_apply_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    const time_t snapshot_timestamp_ms = snapshot->timestamp_ms;

    if(snapshot_timestamp_ms <= instance->last_known_snapshot.timestamp_ms) {
        // Ignore snapshots that are older than the last known one
        FURI_LOG_D(TAG, "Ignoring stale/own snapshot with timestamp %llu", snapshot_timestamp_ms);
        return;
    }

    const BusyTimerState old_state = instance->state;

    busy_timer_stop_timer(instance);

    BusyTimerMode new_mode;
    BusyTimerState new_state;

    const BusyTimerSnapshotType type = snapshot->type;

    if(type == BusyTimerSnapshotTypeNotStarted) {
        new_mode = BusyTimerModeMax;
        new_state = BusyTimerStateIdle;

    } else if(type == BusyTimerSnapshotTypeInfinite) {
        new_mode = BusyTimerModeInfinite;
        new_state = BusyTimerStateWork;

    } else if(type == BusyTimerSnapshotTypeSimple) {
        const BusyTimerSnapshotSimple* simple = &snapshot->simple;

        new_mode = BusyTimerModeSimple;
        new_state = BusyTimerStateWork;

        instance->time_elapsed_s = 0;
        instance->time_remaining_s = MS_TO_S(simple->time_left_ms);

    } else if(type == BusyTimerSnapshotTypeInterval) {
        const BusyTimerSnapshotInterval* interval = &snapshot->interval;
        const BusyTimerIntervalState* interval_state = &interval->state;

        new_mode = BusyTimerModeInterval;
        new_state = interval_state->index % 2 ? BusyTimerStateRest : BusyTimerStateWork;

        instance->current_interval_index = interval_state->index;
        instance->time_elapsed_s =
            MS_TO_S(interval_state->time_total_ms - interval_state->time_left_ms);
        instance->time_remaining_s = MS_TO_S(interval_state->time_left_ms);
        instance->timer_config.interval = interval->config;

    } else {
        furi_crash("Invalid BusyTimerSnapshotType value");
    }

    instance->last_known_snapshot = *snapshot;

    instance->timer_config.mode = new_mode;
    instance->state = new_state;

    strcpy(instance->card_id, snapshot->common.card_id);
    instance->app_config = snapshot->app_config;

    if(new_state != BusyTimerStateIdle) {
        if(old_state != BusyTimerStateIdle) {
            busy_timer_notify_session_ended(instance, BusyTimerSessionOutcomeStopped);
        }

        if(!snapshot->common.is_paused) {
            busy_timer_start_timer(instance);
        }
        // Override prev_tick_timestamp_ms value set by busy_timer_start_timer() call above
        instance->prev_tick_timestamp_ms = snapshot_timestamp_ms;

        busy_timer_start_app(&snapshot->app_config);
        busy_timer_notify_initial_state(instance);
        busy_timer_notify_session_started(instance);

    } else {
        if(old_state != BusyTimerStateIdle) {
            busy_timer_notify_session_ended(instance, BusyTimerSessionOutcomeStopped);
        }

        busy_timer_notify_state_changed(instance);
        busy_timer_exit_app();
    }

    busy_timer_schedule_publish_last_known_snapshot(instance);
}

static BusyTimerSetProfileResult busy_timer_set_profile_internal(
    BusyTimer* instance,
    const BusyTimerProfile* profile,
    BusyTimerProfileId profile_id) {
    BusyTimerSetProfileResult result;

    do {
        const time_t profile_timestamp_ms = profile->timestamp_ms;

        if(!busy_timer_profile_is_valid(profile)) {
            FURI_LOG_E(TAG, "Ignoring invalid profile with timestamp %llu", profile_timestamp_ms);
            result = BusyTimerSetProfileResultRejectedInvalid;
            break;
        }

        BusyTimerProfile* current_profile = &instance->settings[profile_id].profile;
        const time_t current_timestamp_ms = current_profile->timestamp_ms;

        if(profile_timestamp_ms < current_timestamp_ms) {
            FURI_LOG_D(TAG, "Ignoring stale profile with timestamp %llu", profile_timestamp_ms);
            result = BusyTimerSetProfileResultRejectedOutdated;
            break;

        } else if(profile_timestamp_ms == current_timestamp_ms) {
            FURI_LOG_D(TAG, "Ignoring own profile with timestamp %llu", profile_timestamp_ms);
            result = BusyTimerSetProfileResultRejectedOwn;
            break;
        }

        *current_profile = *profile;

        const time_t now_timestamp_ms = furi_hal_rtc_get_timestamp_ms();

        if(profile_timestamp_ms > now_timestamp_ms) {
            FURI_LOG_W(TAG, "Profile from the future with timestamp %llu", profile_timestamp_ms);
            current_profile->timestamp_ms = now_timestamp_ms;
        }

        result = BusyTimerSetProfileResultAccepted;

    } while(false);

    return result;
}

static void busy_timer_poll_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;
    busy_timer_update(instance, furi_hal_rtc_get_timestamp_ms());
}

static void busy_timer_snapshot_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;

    if(instance->snapshot_update_count > 1) {
        busy_timer_publish_last_known_snapshot(instance);
        busy_timer_store_saved_state(instance);
    }

    instance->snapshot_update_count = 0;
}

static void busy_timer_profile_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;

    for(BusyTimerProfileId id = 0; id < BusyTimerProfileIdMax; ++id) {
        if(instance->profile_update_count[id] > 1) {
            busy_timer_publish_profile(instance, id);
        }

        instance->profile_update_count[id] = 0;
    }
}

static void busy_timer_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyTimer* instance = context;
    furi_assert(instance->api_queue == object);

    BusyTimerApiMessage message;
    while(furi_message_queue_get(instance->api_queue, &message, 0) == FuriStatusOk) {
        furi_assert(message.type < BusyTimerApiMessageTypeMax);

        busy_timer_api_message_handlers[message.type](instance, &message.data);

        if(message.lock) {
            api_lock_unlock(message.lock);
        }
    }
}

static void busy_timer_mqtt_snapshot_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    BusyTimer* instance = context;

    size_t json_text_len;
    const char* json_text = mqtt_message_get_data(message, &json_text_len);

    BusyTimerSnapshot snapshot;
    if(busy_timer_snapshot_deserialize(&snapshot, json_text, json_text_len)) {
        busy_timer_set_snapshot(instance, &snapshot, BusyTimerSessionSourceIntegrationMqtt);
    } else {
        FURI_LOG_W(TAG, "Invalid snapshot data");
    }
}

static void busy_timer_mqtt_profile_busy_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    BusyTimer* instance = context;

    size_t json_text_len;
    const char* json_text = mqtt_message_get_data(message, &json_text_len);

    BusyTimerProfile profile;
    if(busy_timer_profile_deserialize(&profile, json_text, json_text_len)) {
        busy_timer_set_profile(instance, BusyTimerProfileIdBusy, &profile);
    } else {
        FURI_LOG_W(TAG, "Invalid busy profile data");
    }
}

static void busy_timer_mqtt_profile_custom_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    BusyTimer* instance = context;

    size_t json_text_len;
    const char* json_text = mqtt_message_get_data(message, &json_text_len);

    BusyTimerProfile profile;
    if(busy_timer_profile_deserialize(&profile, json_text, json_text_len)) {
        busy_timer_set_profile(instance, BusyTimerProfileIdCustom, &profile);
    } else {
        FURI_LOG_W(TAG, "Invalid custom profile data");
    }
}

// Private API

void busy_timer_apply_profile_settings(BusyTimer* instance, BusyTimerProfileId profile_id) {
    const BusyTimerSettings* settings = &instance->settings[profile_id];
    const BusyTimerProfile* profile = &settings->profile;

    instance->app_config = profile->app_config;
    instance->timer_config = profile->timer_config;
    strcpy(instance->card_id, profile->metadata.card_id);
    instance->is_demo_mode_enabled = settings->is_demo_mode_enabled;
}

void busy_timer_start_internal(BusyTimer* instance) {
    if(instance->state == BusyTimerStateIdle) {
        busy_timer_notify_mode_changed(instance);
        busy_timer_next_state(instance, true, BusyTimerSessionOutcomeCompleted);
        busy_timer_notify_session_started(instance);

        FURI_LOG_I(TAG, "Started");

    } else {
        busy_timer_start_timer(instance);
        busy_timer_notify_initial_state(instance);

        FURI_LOG_I(TAG, "Resumed");
    }

    busy_timer_capture_and_publish_snapshot(instance);
}

void busy_timer_stop_internal(BusyTimer* instance) {
    if(instance->state != BusyTimerStateIdle) {
        instance->state = BusyTimerStateIdle;
        busy_timer_stop_timer(instance);
        busy_timer_notify_state_changed(instance);
        busy_timer_notify_session_ended(instance, BusyTimerSessionOutcomeStopped);

        FURI_LOG_I(TAG, "Stopped");

        busy_timer_capture_and_publish_snapshot(instance);
    }
}

void busy_timer_toggle_internal(BusyTimer* instance) {
    if(busy_timer_is_running(instance)) {
        busy_timer_stop_timer(instance);
        FURI_LOG_I(TAG, "Paused");

    } else {
        busy_timer_start_timer(instance);
        FURI_LOG_I(TAG, "Resumed");
    }

    busy_timer_notify_paused(instance);

    busy_timer_capture_and_publish_snapshot(instance);
}

void busy_timer_skip_internal(BusyTimer* instance) {
    if(busy_timer_is_running(instance)) {
        busy_timer_next_state(instance, true, BusyTimerSessionOutcomeStopped);

        busy_timer_capture_and_publish_snapshot(instance);

        FURI_LOG_I(TAG, "Skipped");
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

static void
    busy_timer_start_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    if(instance->state == BusyTimerStateIdle) {
        const BusyTimerProfileId profile_id = data->start.profile_id;
        instance->active_profile_id = profile_id;
        busy_timer_apply_profile_settings(instance, profile_id);
    }

    instance->session_source = data->start.source;
    busy_timer_start_internal(instance);
}

static void
    busy_timer_stop_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    UNUSED(data);
    busy_timer_stop_internal(instance);
}

static void
    busy_timer_add_time_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    if(!busy_timer_is_running(instance)) {
        // Ignore if the timer is not running (paused)
        return;
    }

    const int32_t add_time_minutes = data->add_time.time_minutes;

    if(instance->timer_config.mode == BusyTimerModeInfinite) {
        if(add_time_minutes > 0) {
            // Special case: start a Simple timer
            busy_timer_infinite_to_simple(instance);
            busy_timer_capture_and_publish_snapshot(instance);
        }
        return;
    }

    int32_t time_remaining_s = instance->time_remaining_s;
    int32_t increment_s = M_TO_S(add_time_minutes);

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

    instance->time_remaining_s =
        CLAMP(time_remaining_s, BUSY_TIMER_TIME_MAX_S, BUSY_TIMER_TIME_MIN_S);

    busy_timer_start_timer(instance);
    busy_timer_notify_tick(instance);

    busy_timer_capture_and_publish_snapshot(instance);

    FURI_LOG_I(TAG, "Interval override");
}

static void
    busy_timer_toggle_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    UNUSED(data);
    busy_timer_toggle_internal(instance);
}

static void
    busy_timer_skip_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    UNUSED(data);
    busy_timer_skip_internal(instance);
}

static void
    busy_timer_finalize_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    UNUSED(data);

    if(instance->state == BusyTimerStateIdle) {
        busy_timer_capture_and_publish_snapshot(instance);

        FURI_LOG_I(TAG, "Finalized");
    }
}

static void busy_timer_get_run_info_api_message_handler(
    BusyTimer* instance,
    BusyTimerApiMessageData* data) {
    BusyTimerRunInfo* timer_info = data->get_run_info.run_info;
    timer_info->state = instance->state;
    timer_info->config = instance->timer_config;
    timer_info->current_interval_idx = instance->current_interval_index;
    timer_info->time_elapsed_s = instance->time_elapsed_s;
    timer_info->session_source = instance->session_source;
}

static void busy_timer_get_snapshot_api_message_handler(
    BusyTimer* instance,
    BusyTimerApiMessageData* data) {
    const BusyTimerApiMessageGetSnapshot* get_snapshot = &data->get_snapshot;
    *get_snapshot->snapshot = instance->last_known_snapshot;
}

static void busy_timer_set_snapshot_api_message_handler(
    BusyTimer* instance,
    BusyTimerApiMessageData* data) {
    const BusyTimerApiMessageSetSnapshot* set_snapshot = &data->set_snapshot;
    instance->session_source = set_snapshot->source;
    instance->active_profile_id = BusyTimerProfileIdMax;
    busy_timer_apply_snapshot(instance, &set_snapshot->snapshot);
}

static void
    busy_timer_get_profile_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    const BusyTimerApiMessageGetProfile* get_profile = &data->get_profile;

    BusyTimerProfile* const profile = get_profile->profile;
    const BusyTimerProfileId profile_id = get_profile->profile_id;

    const BusyTimerProfile* current_profile = &instance->settings[profile_id].profile;
    *profile = *current_profile;
}

static void
    busy_timer_set_profile_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    const BusyTimerApiMessageSetProfile* set_profile = &data->set_profile;

    const BusyTimerProfile* profile = &set_profile->profile;
    const BusyTimerProfileId profile_id = set_profile->profile_id;

    const BusyTimerSetProfileResult result =
        busy_timer_set_profile_internal(instance, profile, profile_id);

    if(result == BusyTimerSetProfileResultAccepted) {
        busy_timer_settings_save(&instance->settings[profile_id], profile_id);
    }
    // Do not re-publish upon receiving invalid or own (same) profile
    if(result != BusyTimerSetProfileResultRejectedInvalid &&
       result != BusyTimerSetProfileResultRejectedOwn) {
        busy_timer_schedule_publish_profile(instance, profile_id);
    }
}

static void
    busy_timer_get_preset_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    const BusyTimerApiMessageGetPreset* get_preset = &data->get_preset;

    const BusyTimerProfileId profile_id = get_preset->profile_id;
    furi_assert(profile_id < BusyTimerProfileIdMax);

    BusyTimerPreset* preset = get_preset->preset;
    const BusyTimerSettings* settings = &instance->settings[profile_id];
    const BusyTimerProfile* profile = &settings->profile;

    preset->app_config = profile->app_config;
    preset->timer_config = profile->timer_config;
    preset->is_demo_mode_enabled = settings->is_demo_mode_enabled;
}

static void
    busy_timer_set_preset_api_message_handler(BusyTimer* instance, BusyTimerApiMessageData* data) {
    const BusyTimerApiMessageSetPreset* set_preset = &data->set_preset;

    const BusyTimerProfileId profile_id = set_preset->profile_id;
    furi_assert(profile_id < BusyTimerProfileIdMax);

    const BusyTimerPreset* preset = &set_preset->preset;

    BusyTimerSettings* settings = &instance->settings[profile_id];
    settings->is_demo_mode_enabled = preset->is_demo_mode_enabled;

    BusyTimerProfile* profile = &settings->profile;
    profile->app_config = preset->app_config;
    profile->timer_config = preset->timer_config;
    profile->timestamp_ms = furi_hal_rtc_get_timestamp_ms();

    busy_timer_settings_save(&instance->settings[profile_id], profile_id);
    busy_timer_schedule_publish_profile(instance, profile_id);
}

static void busy_timer_handle_matter_api_message_handler(
    BusyTimer* instance,
    BusyTimerApiMessageData* data) {
    MatterSwitchState switch_state = data->handle_matter.switch_state;
    busy_timer_smart_home_handle_switch_state(instance, switch_state);
}

// Service

static void busy_timer_load_settings(BusyTimer* instance) {
    for(BusyTimerProfileId id = 0; id < BusyTimerProfileIdMax; ++id) {
        busy_timer_settings_load(&instance->settings[id], id);
    }
}

static void busy_timer_load_saved_state(BusyTimer* instance) {
    busy_timer_saved_state_load(&instance->saved_state);
    busy_timer_set_snapshot(
        instance, &instance->saved_state.snapshot, BusyTimerSessionSourceUnknown);
}

static BusyTimer* busy_timer_alloc(void) {
    BusyTimer* instance = malloc(sizeof(BusyTimer));

    instance->event_loop = furi_event_loop_alloc();
    instance->poll_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_timer_poll_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->snapshot_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_timer_snapshot_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->profile_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        busy_timer_profile_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->api_queue = furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(BusyTimerApiMessage));
    instance->event_pubsub = furi_pubsub_alloc();
    instance->mqtt = furi_record_open(RECORD_MQTT);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        busy_timer_message_queue_callback,
        instance);

    mqtt_subscribe(
        instance->mqtt,
        TIMER_SNAPSHOT_MQTT_QOS,
        TIMER_SNAPSHOT_MQTT_TOPIC,
        busy_timer_mqtt_snapshot_callback,
        instance);

    mqtt_subscribe(
        instance->mqtt,
        TIMER_PROFILE_MQTT_QOS,
        busy_timer_mqtt_topics[BusyTimerProfileIdBusy],
        busy_timer_mqtt_profile_busy_callback,
        instance);

    mqtt_subscribe(
        instance->mqtt,
        TIMER_PROFILE_MQTT_QOS,
        busy_timer_mqtt_topics[BusyTimerProfileIdCustom],
        busy_timer_mqtt_profile_custom_callback,
        instance);

    busy_timer_smart_home_init(instance);
    busy_timer_status_lights_init(instance);

    busy_timer_load_settings(instance);
    busy_timer_load_saved_state(instance);

    furi_record_create(RECORD_BUSY_TIMER, instance);

    return instance;
}

int busy_timer_srv(void* arg) {
    UNUSED(arg);

    BusyTimer* instance = busy_timer_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const BusyTimerApiMessageHandler
    busy_timer_api_message_handlers[BusyTimerApiMessageTypeMax] = {
        [BusyTimerApiMessageTypeStart] = busy_timer_start_api_message_handler,
        [BusyTimerApiMessageTypeStop] = busy_timer_stop_api_message_handler,
        [BusyTimerApiMessageTypeAddTime] = busy_timer_add_time_api_message_handler,
        [BusyTimerApiMessageTypeToggle] = busy_timer_toggle_api_message_handler,
        [BusyTimerApiMessageTypeSkip] = busy_timer_skip_api_message_handler,
        [BusyTimerApiMessageTypeFinalize] = busy_timer_finalize_api_message_handler,
        [BusyTimerApiMessageTypeGetRunInfo] = busy_timer_get_run_info_api_message_handler,
        [BusyTimerApiMessageTypeGetSnapshot] = busy_timer_get_snapshot_api_message_handler,
        [BusyTimerApiMessageTypeSetSnapshot] = busy_timer_set_snapshot_api_message_handler,
        [BusyTimerApiMessageTypeGetProfile] = busy_timer_get_profile_api_message_handler,
        [BusyTimerApiMessageTypeSetProfile] = busy_timer_set_profile_api_message_handler,
        [BusyTimerApiMessageTypeGetPreset] = busy_timer_get_preset_api_message_handler,
        [BusyTimerApiMessageTypeSetPreset] = busy_timer_set_preset_api_message_handler,
        [BusyTimerApiMessageTypeHandleMatter] = busy_timer_handle_matter_api_message_handler,
};
