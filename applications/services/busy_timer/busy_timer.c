#include "busy_timer_i.h"

#include <busy/busy.h>
#include <desktop/desktop.h>

#include <furi_hal_rtc.h>

#ifdef BUSY_TIMER_TICK_DEBUG
#define TIME_MAX_LEN (14)
#endif

#define POLL_TIMER_PERIOD_MS    (S_TO_MS(1) / 30)
#define DEBOUNCE_TIMER_DELAY_MS (S_TO_MS(1))

#define TIMER_SNAPSHOT_MQTT_TOPIC "busy/snapshot"
#define TIMER_SNAPSHOT_MQTT_QOS   MqttQosAtLeastOnce

#define TIMER_PROFILE_MQTT_QOS          MqttQosAtLeastOnce
#define TIMER_PROFILE_BUSY_MQTT_TOPIC   "busy/profiles/busy"
#define TIMER_PROFILE_CUSTOM_MQTT_TOPIC "busy/profiles/custom"

typedef void (*const BusyTimerMessageHandler)(BusyTimer* instance, BusyTimerMessageData* data);

static const BusyTimerMessageHandler busy_timer_message_handlers[];

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

static void busy_timer_start_app(const BusyAppConfig* app_config) {
    if(!furi_record_exists(RECORD_BUSY_APP)) {
        Desktop* desktop = furi_record_open(RECORD_DESKTOP);
        while(!desktop_replace_current_app(desktop, "busy", BUSY_APP_TIMER_MODE)) {
            furi_thread_yield();
        }
        furi_record_close(RECORD_DESKTOP);
    }

    BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);

    busy_set_config(busy_app, app_config);
    busy_show_timer(busy_app);

    furi_record_close(RECORD_BUSY_APP);
}

static void busy_timer_exit_app(void) {
    if(furi_record_exists(RECORD_BUSY_APP)) {
        BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);
        busy_request_exit(busy_app);
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
        .type = BusyTimerEventTypePaused,
        .paused.is_paused = !instance->timer_running,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static void
    busy_timer_notify_profile_changed(const BusyTimer* instance, BusyTimerProfileId profile_id) {
    FURI_LOG_D(TAG, "Profile changed: %d", profile_id);

    BusyTimerEvent event = {
        .type = BusyTimerEventTypeProfileChanged,
        .profile_changed.profile_id = profile_id,
    };

    furi_pubsub_publish(instance->event_pubsub, &event);
}

static BusyTimerState busy_timer_calc_next_state(const BusyTimer* instance) {
    BusyTimerState next_state;

    const BusyTimerMode timer_mode = instance->mode;

    if(instance->state == BusyTimerStateIdle) {
        next_state = BusyTimerStateWork;

    } else if(instance->state == BusyTimerStateWork) {
        if(timer_mode == BusyTimerModeInterval) {
            const uint32_t cycles_count = instance->interval_config.cycles_count;

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

    if(instance->mode == BusyTimerModeInfinite) {
        interval_s = 0;

    } else if(instance->mode == BusyTimerModeSimple) {
        const BusyTimerSimpleConfig* simple = &instance->simple_config;

        if(instance->state == BusyTimerStateWork) {
            interval_s = MS_TO_S(simple->total_time_ms);
        } else {
            furi_crash("Invalid timer state in simple mode");
        }

    } else if(instance->mode == BusyTimerModeInterval) {
        const BusyTimerIntervalConfig* interval = &instance->interval_config;

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
        if(instance->mode == BusyTimerModeInterval) {
            interval_index = instance->current_interval_index + 1;
        }
    }

    return interval_index;
}

static uint32_t busy_timer_calc_increment(const BusyTimer* instance) {
    UNUSED(instance);
    // TODO: Figure out demo mode?
    /*
    if(instance->settings.timer_config.enable_demo_mode) {
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
    */
    return 1;
}

static bool busy_timer_is_running(const BusyTimer* instance) {
    return instance->timer_running;
}

static void busy_timer_start_timer(BusyTimer* instance) {
    if(instance->mode != BusyTimerModeInfinite) {
        furi_event_loop_timer_start(instance->poll_timer, POLL_TIMER_PERIOD_MS);
    }

    instance->prev_tick_timestamp_ms = furi_hal_rtc_get_timestamp_ms();
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

static void busy_timer_next_state(BusyTimer* instance, bool is_forced) {
    FURI_LOG_I(TAG, "Current state: %s", busy_timer_get_state_name(instance->state));

    instance->current_interval_index = busy_timer_calc_interval_index(instance);
    instance->state = busy_timer_calc_next_state(instance);

    if(instance->state != BusyTimerStateIdle) {
        instance->time.elapsed_s = 0;
        instance->time.remain_s = busy_timer_calc_remaining_time(instance);

        bool is_autostart_enabled = false;

        if(instance->mode == BusyTimerModeInterval) {
            is_autostart_enabled = instance->interval_config.is_autostart_enabled;
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

static void busy_timer_fill_snapshot_common(BusyTimer* instance, BusyTimerSnapshotCommon* common) {
    strcpy(common->card_id, instance->card_id);
    common->is_paused = !busy_timer_is_running(instance);
}

static void busy_timer_make_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    snapshot->timestamp_ms = furi_hal_rtc_get_timestamp_ms();

    if(instance->state != BusyTimerStateIdle) {
        const BusyTimerMode timer_mode = instance->mode;

        if(timer_mode == BusyTimerModeInfinite) {
            snapshot->type = BusyTimerSnapshotTypeInfinite;

            BusyTimerSnapshotInfinite* infinite = &snapshot->infinite;
            busy_timer_fill_snapshot_common(instance, &infinite->common);

        } else if(timer_mode == BusyTimerModeSimple) {
            snapshot->type = BusyTimerSnapshotTypeSimple;

            BusyTimerSnapshotSimple* simple = &snapshot->simple;
            busy_timer_fill_snapshot_common(instance, &simple->common);

            const BusyTimerTime* time = &instance->time;
            simple->time_left_ms = S_TO_MS(time->remain_s);

        } else if(timer_mode == BusyTimerModeInterval) {
            snapshot->type = BusyTimerSnapshotTypeInterval;

            BusyTimerSnapshotInterval* interval = &snapshot->interval;
            busy_timer_fill_snapshot_common(instance, &interval->common);

            const BusyTimerTime* time = &instance->time;
            BusyTimerIntervalState* state = &interval->state;
            state->index = instance->current_interval_index;
            state->time_left_ms = S_TO_MS(time->remain_s);
            state->time_total_ms = S_TO_MS(time->elapsed_s + time->remain_s);

            interval->config = instance->interval_config;

        } else {
            furi_crash("Invalid BusyTimerMode value");
        }

    } else {
        snapshot->type = BusyTimerSnapshotTypeNotStarted;
    }

    snapshot->app_config = instance->app_config;
}

static void busy_timer_apply_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    const time_t snapshot_timestamp_ms = snapshot->timestamp_ms;

    if(snapshot_timestamp_ms <= instance->user_snapshot.timestamp_ms) {
        // Ignore snapshots that are older than the last known one
        FURI_LOG_D(TAG, "Ignoring stale/own snapshot with timestamp %llu", snapshot_timestamp_ms);
        return;
    }

    if(!busy_timer_snapshot_is_valid(snapshot)) {
        FURI_LOG_W(TAG, "Ignoring invalid snapshot with timestamp %llu", snapshot_timestamp_ms);
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

        new_mode = BusyTimerModeInterval;
        new_state = interval_state->index % 2 ? BusyTimerStateRest : BusyTimerStateWork;

        instance->current_interval_index = interval_state->index;
        instance->time.elapsed_s =
            MS_TO_S(interval_state->time_total_ms - interval_state->time_left_ms);
        instance->time.remain_s = MS_TO_S(interval_state->time_left_ms);
        instance->interval_config = interval->config;

    } else {
        furi_crash("Invalid BusyTimerSnapshotType value");
    }

    instance->mode = new_mode;
    instance->state = new_state;

    strcpy(instance->card_id, snapshot->common.card_id);
    instance->app_config = snapshot->app_config;

    if(!snapshot->common.is_paused) {
        busy_timer_start_timer(instance);
    }

    instance->prev_tick_timestamp_ms = snapshot_timestamp_ms;

    busy_timer_start_app(&snapshot->app_config);

    busy_timer_notify_mode_changed(instance);
    busy_timer_notify_state_changed(instance);
    busy_timer_notify_tick(instance);
    busy_timer_notify_paused(instance);
}

static void busy_timer_schedule_send_snapshot(BusyTimer* instance) {
    busy_timer_make_snapshot(instance, &instance->user_snapshot);
    furi_event_loop_timer_start(instance->debounce_timer, DEBOUNCE_TIMER_DELAY_MS);
}

static void busy_timer_load_profile_from_settings(
    BusyTimerProfile* profile,
    BusyTimerProfileId profile_id) {
    BusyTimerSettings settings;
    busy_timer_settings_load(&settings, profile_id);

    profile->app_config = settings.app_config;
    profile->timer_config = settings.timer_config;
    profile->metadata = settings.metadata;
    profile->timestamp_ms = settings.timestamp_ms;
}

static void busy_timer_save_profile_to_settings(
    const BusyTimerProfile* profile,
    BusyTimerProfileId profile_id) {
    BusyTimerSettings settings;

    settings.app_config = profile->app_config;
    settings.timer_config = profile->timer_config;
    settings.metadata = profile->metadata;
    settings.timestamp_ms = profile->timestamp_ms;

    busy_timer_settings_save(&settings, profile_id);
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

        const time_t max_future_timestamp_ms = furi_hal_rtc_get_timestamp_ms() + M_TO_MS(10);

        if(profile_timestamp_ms > max_future_timestamp_ms) {
            FURI_LOG_W(
                TAG, "Ignoring profile from future with timestamp %llu", profile_timestamp_ms);
            result = BusyTimerSetProfileResultRejectedFuture;
            break;
        }

        BusyTimerProfile* current_profile = &instance->profiles[profile_id];
        const time_t current_profile_timestamp_ms = current_profile->timestamp_ms;

        if(profile_timestamp_ms < current_profile_timestamp_ms) {
            FURI_LOG_D(TAG, "Ignoring stale profile with timestamp %llu", profile_timestamp_ms);
            result = BusyTimerSetProfileResultRejectedOutdated;
            break;

        } else if(profile_timestamp_ms == current_profile_timestamp_ms) {
            FURI_LOG_D(TAG, "Ignoring own profile with timestamp %llu", profile_timestamp_ms);
            result = BusyTimerSetProfileResultRejectedOwn;
            break;
        }

        *current_profile = *profile;
        busy_timer_save_profile_to_settings(current_profile, profile_id);

        result = BusyTimerSetProfileResultAccepted;
    } while(false);

    return result;
}

static void busy_timer_publish_profile(BusyTimer* instance, BusyTimerProfileId profile_id) {
    const BusyTimerProfile* profile = &instance->profiles[profile_id];

    char* profile_str = busy_timer_profile_serialize(profile);
    furi_check(profile_str);

    const char* mqtt_topic;

    if(profile_id == BusyTimerProfileIdBusy) {
        mqtt_topic = TIMER_PROFILE_BUSY_MQTT_TOPIC;
    } else if(profile_id == BusyTimerProfileIdCustom) {
        mqtt_topic = TIMER_PROFILE_CUSTOM_MQTT_TOPIC;
    } else {
        furi_crash("Invalid BusyTimerProfileId value");
    }

    mqtt_publish(
        instance->mqtt, TIMER_PROFILE_MQTT_QOS, mqtt_topic, profile_str, strlen(profile_str));

    free(profile_str);
}

static void busy_timer_poll_timer_callback(void* context) {
    furi_assert(context);
    BusyTimer* instance = context;
    busy_timer_update(instance, furi_hal_rtc_get_timestamp_ms());
}

static void busy_timer_debounce_timer_callback(void* context) {
    furi_assert(context);
    const BusyTimer* instance = context;

    char* snapshot_str = busy_timer_snapshot_serialize(&instance->user_snapshot);
    furi_check(snapshot_str);

    mqtt_publish(
        instance->mqtt,
        TIMER_SNAPSHOT_MQTT_QOS,
        TIMER_SNAPSHOT_MQTT_TOPIC,
        snapshot_str,
        strlen(snapshot_str));

    free(snapshot_str);
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

static void busy_timer_mqtt_shapshot_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    BusyTimer* instance = context;

    size_t json_text_len;
    const char* json_text = mqtt_message_get_data(message, &json_text_len);

    BusyTimerSnapshot snapshot;
    if(busy_timer_snapshot_deserialize(&snapshot, json_text, json_text_len)) {
        busy_timer_set_snapshot(instance, &snapshot);
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
        FURI_LOG_W(TAG, "Invalid profile data");
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
        FURI_LOG_W(TAG, "Invalid profile data");
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

    busy_timer_schedule_send_snapshot(instance);
}

static void busy_timer_stop_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    busy_timer_stop_timer(instance);
    instance->state = BusyTimerStateIdle;

    FURI_LOG_I(TAG, "Stopped");

    busy_timer_schedule_send_snapshot(instance);
}

static void busy_timer_get_state_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    *data->state = instance->state;
}

static void
    busy_timer_get_cycles_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    data->cycles->total_count = instance->interval_config.cycles_count;
    data->cycles->current_idx = instance->current_interval_index;
}

static void busy_timer_get_info_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    BusyTimerInfo* timer_info = data->get_info.info;
    BusyTimerConfig* timer_config = &timer_info->config;

    const BusyTimerMode timer_mode = instance->mode;
    timer_config->mode = timer_mode;

    if(timer_mode == BusyTimerModeSimple) {
        timer_config->simple = instance->simple_config;
    } else if(timer_mode == BusyTimerModeInterval) {
        timer_config->interval = instance->interval_config;
    }
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
    busy_timer_schedule_send_snapshot(instance);

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
    busy_timer_schedule_send_snapshot(instance);
}

static void busy_timer_skip_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    UNUSED(data);

    if(busy_timer_is_running(instance)) {
        busy_timer_next_state(instance, true);
        busy_timer_schedule_send_snapshot(instance);
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

static void
    busy_timer_get_profile_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    const BusyTimerMessageGetProfile* get_profile = &data->get_profile;

    BusyTimerProfile* const profile = get_profile->profile;
    const BusyTimerProfileId profile_id = get_profile->profile_id;

    *profile = instance->profiles[profile_id];
}

static void
    busy_timer_set_profile_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    const BusyTimerMessageSetProfile* set_profile = &data->set_profile;

    const BusyTimerProfile* profile = set_profile->profile;
    const BusyTimerProfileId profile_id = set_profile->profile_id;

    const BusyTimerSetProfileResult result =
        busy_timer_set_profile_internal(instance, profile, profile_id);
    furi_check(result < BusyTimerSetProfileResultMax);

    if(result == BusyTimerSetProfileResultAccepted) {
        busy_timer_notify_profile_changed(instance, profile_id);
    }
    // Do not re-publish own profiles to avoid infinite loops
    if(result != BusyTimerSetProfileResultRejectedOwn) {
        busy_timer_publish_profile(instance, profile_id);
    }
}

static void
    busy_timer_load_profile_message_handler(BusyTimer* instance, BusyTimerMessageData* data) {
    const BusyTimerProfileId profile_id = data->profile_id;
    furi_assert(profile_id < BusyTimerProfileIdMax);

    const BusyTimerProfile* profile = &instance->profiles[profile_id];

    instance->app_config = profile->app_config;
    strcpy(instance->card_id, profile->metadata.card_id);

    const BusyTimerConfig* timer_config = &profile->timer_config;
    instance->mode = timer_config->mode;

    if(instance->mode == BusyTimerModeSimple) {
        instance->simple_config = timer_config->simple;
    } else if(instance->mode == BusyTimerModeInterval) {
        instance->interval_config = timer_config->interval;
    }
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
    instance->mqtt = furi_record_open(RECORD_MQTT);

    for(BusyTimerProfileId id = 0; id < BusyTimerProfileIdMax; ++id) {
        busy_timer_load_profile_from_settings(&instance->profiles[id], id);
    }

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        busy_timer_message_queue_callback,
        instance);

    mqtt_subscribe(
        instance->mqtt,
        TIMER_SNAPSHOT_MQTT_QOS,
        TIMER_SNAPSHOT_MQTT_TOPIC,
        busy_timer_mqtt_shapshot_callback,
        instance);

    mqtt_subscribe(
        instance->mqtt,
        TIMER_PROFILE_MQTT_QOS,
        TIMER_PROFILE_BUSY_MQTT_TOPIC,
        busy_timer_mqtt_profile_busy_callback,
        instance);

    mqtt_subscribe(
        instance->mqtt,
        TIMER_PROFILE_MQTT_QOS,
        TIMER_PROFILE_CUSTOM_MQTT_TOPIC,
        busy_timer_mqtt_profile_custom_callback,
        instance);

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
    [BusyTimerMessageTypeGetState] = busy_timer_get_state_message_handler,
    [BusyTimerMessageTypeGetCycles] = busy_timer_get_cycles_message_handler,
    [BusyTimerMessageTypeGetInfo] = busy_timer_get_info_message_handler,
    [BusyTimerMessageTypeAddTime] = busy_timer_add_time_message_handler,
    [BusyTimerMessageTypeToggle] = busy_timer_toggle_message_handler,
    [BusyTimerMessageTypeSkip] = busy_timer_skip_message_handler,
    [BusyTimerMessageTypeGetSnapshot] = busy_timer_get_snapshot_message_handler,
    [BusyTimerMessageTypeSetSnapshot] = busy_timer_set_snapshot_message_handler,
    [BusyTimerMessageTypeGetProfile] = busy_timer_get_profile_message_handler,
    [BusyTimerMessageTypeSetProfile] = busy_timer_set_profile_message_handler,
    [BusyTimerMessageTypeLoadProfile] = busy_timer_load_profile_message_handler,
};
