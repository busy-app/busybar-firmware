#include "telemetry_i.h"
#include "telemetry_settings.h"

#include <time/time.h>
#include <version.h>
#include <furi_hal_version.h>
#include <furi_hal_nvm.h>
#include <toolbox/hex.h>

#include <power/power_service/power.h>
#include <audio/audio.h>
#include <matter/matter.h>

// ===== Event registry =====

typedef struct {
    const char* name;
    TelemetryPriority priority;
} TelemetryEventInfo;

static const TelemetryEventInfo telemetry_event_info[TelemetryEventMax] = {
    [TelemetryEventDeviceBoot] = {"device.boot", TelemetryPriorityPush},
    [TelemetryEventFwUpdate] = {"fw.update", TelemetryPriorityPush},
    [TelemetryEventTimerSessionStart] = {"timer.session.start", TelemetryPriorityBatch},
    [TelemetryEventTimerSessionEnd] = {"timer.session.end", TelemetryPriorityPush},
    [TelemetryEventTimerTheme] = {"timer.theme", TelemetryPriorityLow},
    [TelemetryEventAppStart] = {"app.start", TelemetryPriorityBatch},
    [TelemetryEventAppStop] = {"app.stop", TelemetryPriorityBatch},
    [TelemetryEventSettingBrightness] = {"setting.brightness", TelemetryPriorityLow},
    [TelemetryEventSettingVolume] = {"setting.volume", TelemetryPriorityLow},
    [TelemetryEventInputSwitch] = {"input.switch", TelemetryPriorityLow},
    [TelemetryEventPowerTransition] = {"power.transition", TelemetryPriorityBatch},
    [TelemetryEventNetOnline] = {"net.online", TelemetryPriorityPush},
    [TelemetryEventNetOffline] = {"net.offline", TelemetryPriorityPush},
    [TelemetryEventNetOfflineDuration] = {"net.offline_duration", TelemetryPriorityPush},
    [TelemetryEventAccountLink] = {"account.link", TelemetryPriorityPush},
    [TelemetryEventAccountUnlink] = {"account.unlink", TelemetryPriorityPush},
    [TelemetryEventCanvasAcquire] = {"canvas.acquire", TelemetryPriorityBatch},
    [TelemetryEventCanvasRelease] = {"canvas.release", TelemetryPriorityBatch},
};

static_assert(COUNT_OF(telemetry_event_info) == TelemetryEventMax);

const char* telemetry_event_type_name(TelemetryEventType type) {
    furi_check(type < TelemetryEventMax);
    return telemetry_event_info[type].name;
}

// ===== Ring buffer =====

static cJSON* telemetry_ring_pop(Telemetry* instance) {
    furi_assert(instance->events_count > 0);

    cJSON* event = instance->events[instance->events_head];
    instance->events_head = (instance->events_head + 1) % TELEMETRY_RING_CAPACITY;
    instance->events_count--;
    return event;
}

static void telemetry_ring_push(Telemetry* instance, cJSON* event) {
    if(instance->events_count >= TELEMETRY_RING_CAPACITY) {
        // drop the oldest event
        FURI_LOG_W(TAG, "Telemetry ring buffer full, dropping oldest event");
        cJSON_Delete(telemetry_ring_pop(instance));
        instance->events_dropped++;
    }

    const size_t index =
        (instance->events_head + instance->events_count) % TELEMETRY_RING_CAPACITY;
    instance->events[index] = event;
    instance->events_count++;
}

static void telemetry_ring_clear(Telemetry* instance) {
    while(instance->events_count > 0) {
        cJSON_Delete(telemetry_ring_pop(instance));
    }
}

// ===== Event building =====

static cJSON* telemetry_event_create(TelemetryEventType type, cJSON* data) {
    const TelemetryEventInfo* info = &telemetry_event_info[type];

    cJSON* event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "t", info->name);
    cJSON_AddNumberToObject(event, "ts", (double)time_get_timestamp_ms());
    cJSON_AddNumberToObject(event, "p", info->priority);

    if(data) {
        cJSON_AddItemToObject(event, "d", data);
    } else {
        cJSON_AddObjectToObject(event, "d");
    }

    return event;
}

// ===== Composite (flush-time) events =====

static void telemetry_add_device_state(Telemetry* instance, cJSON* events) {
    cJSON* d = cJSON_CreateObject();

    PowerInfo power_info;
    power_get_info(instance->power, &power_info);
    cJSON_AddNumberToObject(d, "charge", power_info.charge);
    cJSON_AddBoolToObject(d, "charging", power_info.is_charging);
    cJSON_AddNumberToObject(d, "charge_limit", power_info.charge_level_limit);

    MatterCommissionedFabrics fabrics;
    if(matter_get_commissioned_fabrics(instance->matter, &fabrics) == MatterStatusOk) {
        cJSON_AddNumberToObject(d, "matter_fabrics", fabrics.count);
        cJSON_AddBoolToObject(d, "matter_commissioned", fabrics.count > 0);
    }

    MqttSessionInfo session_info = {
        .session_id = NULL,
        .user_id = NULL,
        .email = NULL,
    };
    mqtt_get_session_info(instance->mqtt, &session_info);
    cJSON_AddBoolToObject(d, "account_linked", session_info.is_valid);

    cJSON_AddBoolToObject(d, "dev_mode", furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug));

    cJSON* event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "t", "device.state");
    cJSON_AddNumberToObject(event, "ts", (double)time_get_timestamp_ms());
    cJSON_AddNumberToObject(event, "p", TelemetryPriorityLow);
    cJSON_AddItemToObject(event, "d", d);
    cJSON_AddItemToArray(events, event);
}

static void telemetry_add_input_counts(Telemetry* instance, cJSON* events) {
    const uint32_t ok = atomic_exchange(&instance->input_ok, 0);
    const uint32_t back = atomic_exchange(&instance->input_back, 0);
    const uint32_t start = atomic_exchange(&instance->input_start, 0);
    const uint32_t wheel_up = atomic_exchange(&instance->input_wheel_up, 0);
    const uint32_t wheel_down = atomic_exchange(&instance->input_wheel_down, 0);

    if(ok == 0 && back == 0 && start == 0 && wheel_up == 0 && wheel_down == 0) {
        return;
    }

    cJSON* d = cJSON_CreateObject();
    cJSON_AddNumberToObject(d, "ok", ok);
    cJSON_AddNumberToObject(d, "back", back);
    cJSON_AddNumberToObject(d, "start", start);
    cJSON_AddNumberToObject(d, "wheel_up", wheel_up);
    cJSON_AddNumberToObject(d, "wheel_down", wheel_down);

    cJSON* event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "t", "input.counts");
    cJSON_AddNumberToObject(event, "ts", (double)time_get_timestamp_ms());
    cJSON_AddNumberToObject(event, "p", TelemetryPriorityLow);
    cJSON_AddItemToObject(event, "d", d);
    cJSON_AddItemToArray(events, event);
}

// ===== Flush =====

static bool telemetry_push_allowed(Telemetry* instance) {
    const time_t now_ms = time_get_timestamp_ms();
    if(now_ms - instance->last_push_ms >= TELEMETRY_PUSH_MIN_INTERVAL_MS) {
        instance->last_push_ms = now_ms;
        return true;
    }
    return false;
}

static void telemetry_flush(Telemetry* instance, bool is_push) {
    if(!instance->is_enabled) {
        telemetry_ring_clear(instance);
        return;
    }

    if(!instance->is_connected) {
        // Offline: keep buffered p1/p2 events (p0 was dropped at enqueue).
        return;
    }

    if(is_push) {
        if(instance->events_count == 0) {
            return;
        }
        if(!telemetry_push_allowed(instance)) {
            // Rate-limited; the batch will be sent by the next periodic flush.
            return;
        }
    }

    cJSON* batch = cJSON_CreateObject();
    cJSON_AddNumberToObject(batch, "schema", TELEMETRY_SCHEMA);
    cJSON_AddNumberToObject(batch, "ts", (double)time_get_timestamp_ms());
    cJSON* events = cJSON_AddArrayToObject(batch, "events");

    while(instance->events_count > 0) {
        cJSON_AddItemToArray(events, telemetry_ring_pop(instance));
    }

    if(!is_push) {
        telemetry_add_device_state(instance, events);
        telemetry_add_input_counts(instance, events);
    }

    if(cJSON_GetArraySize(events) == 0) {
        cJSON_Delete(batch);
        return;
    }

    char* json = cJSON_PrintUnformatted(batch);
    if(json) {
        FURI_LOG_I(
            TAG,
            "Publishing %u telemetry events (%zu bytes)",
            cJSON_GetArraySize(events),
            strlen(json));
        const bool ok = mqtt_publish_device_scope(
            instance->mqtt, TELEMETRY_MQTT_QOS, TELEMETRY_MQTT_TOPIC, json, strlen(json));
        if(!ok) {
            FURI_LOG_W(TAG, "Failed to publish telemetry batch");
        } else {
            instance->batches_sent++;
            instance->events_sent += cJSON_GetArraySize(events);
        }
        free(json);
    }
    cJSON_Delete(batch);
}

// ===== Enqueue (telemetry thread only) =====

static void
    telemetry_enqueue(Telemetry* instance, TelemetryEventType type, cJSON* data, bool auto_flush) {
    furi_assert(type < TelemetryEventMax);
    instance->events_by_type[type]++;

    if(!instance->is_enabled) {
        instance->events_dropped++;
        if(data) {
            cJSON_Delete(data);
        }
        return;
    }

    const TelemetryEventInfo* info = &telemetry_event_info[type];

    if(!instance->is_connected && info->priority == TelemetryPriorityLow) {
        // p0 events are droppable when offline
        instance->events_dropped++;
        if(data) {
            cJSON_Delete(data);
        }
        return;
    }

    cJSON* event = telemetry_event_create(type, data);
    telemetry_ring_push(instance, event);

    if(!auto_flush) {
        return;
    }

    if(info->priority == TelemetryPriorityPush) {
        telemetry_flush(instance, true);
    } else if(instance->events_count >= TELEMETRY_FLUSH_BATCH_EVENTS) {
        telemetry_flush(instance, false);
    }
}

// ===== API message handlers =====

static void
    telemetry_handle_report_event(Telemetry* instance, const TelemetryApiMessage* message) {
    const TelemetryApiMessageReportEvent* report = &message->data.report_event;
    furi_assert(report->type < TelemetryEventMax);

    telemetry_enqueue(instance, report->type, report->data, true);
}

static void telemetry_handle_mqtt_status(Telemetry* instance, MqttStatus status) {
    const bool connected = (status == MqttStatusConnectedLinked) ||
                           (status == MqttStatusConnectedNotLinked);

    if(connected && !instance->is_connected) {
        // (re)connected: report the offline duration, then flush the backlog
        if(instance->has_offline_start) {
            const time_t now_ms = time_get_timestamp_ms();
            cJSON* d = cJSON_CreateObject();
            cJSON_AddNumberToObject(
                d, "duration_ms", (double)(now_ms - instance->offline_start_ms));
            telemetry_enqueue(instance, TelemetryEventNetOfflineDuration, d, false);
            instance->has_offline_start = false;
        }

        telemetry_enqueue(instance, TelemetryEventNetOnline, NULL, false);
        telemetry_flush(instance, true);

    } else if(!connected && instance->is_connected) {
        instance->offline_start_ms = time_get_timestamp_ms();
        instance->has_offline_start = true;
        telemetry_enqueue(instance, TelemetryEventNetOffline, NULL, true);
    }

    instance->is_connected = connected;
}

static void telemetry_handle_power_event(Telemetry* instance) {
    PowerInfo info;
    power_get_info(instance->power, &info);

    cJSON* d = cJSON_CreateObject();
    cJSON_AddBoolToObject(d, "charging", info.is_charging);
    cJSON_AddNumberToObject(d, "charge", info.charge);
    cJSON_AddNumberToObject(d, "charge_limit", info.charge_level_limit);

    telemetry_enqueue(instance, TelemetryEventPowerTransition, d, true);
}

static void telemetry_handle_audio_event(Telemetry* instance) {
    const float volume = audio_get_volume(instance->audio);

    cJSON* d = cJSON_CreateObject();
    cJSON_AddNumberToObject(d, "volume", (double)volume);

    telemetry_enqueue(instance, TelemetryEventSettingVolume, d, true);
}

static void telemetry_handle_set_enabled(Telemetry* instance, bool enabled) {
    if(instance->is_enabled == enabled) {
        return;
    }

    instance->is_enabled = enabled;

    TelemetrySettings settings = {
        .is_enabled = enabled,
    };
    telemetry_settings_save(&settings);

    if(!enabled) {
        telemetry_ring_clear(instance);
    }
}

static void telemetry_handle_get_stats(Telemetry* instance, const TelemetryApiMessage* message) {
    TelemetryStats* stats = message->data.get_stats.stats;
    furi_assert(stats);

    stats->is_enabled = instance->is_enabled;
    stats->is_connected = instance->is_connected;
    stats->buffered_events = instance->events_count;
    stats->batches_sent = instance->batches_sent;
    stats->events_sent = instance->events_sent;
    stats->events_dropped = instance->events_dropped;
    memcpy(stats->events_by_type, instance->events_by_type, sizeof(stats->events_by_type));
}

// ===== Event loop =====

static void telemetry_flush_timer_callback(void* context) {
    furi_assert(context);
    Telemetry* instance = context;

    telemetry_flush(instance, false);
}

static void telemetry_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Telemetry* instance = context;
    furi_assert(instance->api_queue == object);

    TelemetryApiMessage message;
    while(furi_message_queue_get(instance->api_queue, &message, 0) == FuriStatusOk) {
        switch(message.type) {
        case TelemetryApiMessageTypeReportEvent:
            telemetry_handle_report_event(instance, &message);
            break;
        case TelemetryApiMessageTypeMqttStatus:
            telemetry_handle_mqtt_status(instance, message.data.mqtt_status);
            break;
        case TelemetryApiMessageTypePowerEvent:
            telemetry_handle_power_event(instance);
            break;
        case TelemetryApiMessageTypeAudioEvent:
            telemetry_handle_audio_event(instance);
            break;
        case TelemetryApiMessageTypeSetEnabled:
            telemetry_handle_set_enabled(instance, message.data.is_enabled);
            break;
        case TelemetryApiMessageTypeGetStats:
            telemetry_handle_get_stats(instance, &message);
            break;
        case TelemetryApiMessageTypeFlush:
            telemetry_flush(instance, false);
            break;
        default:
            furi_crash("Unknown telemetry API message type");
            break;
        }

        if(message.lock) {
            api_lock_unlock(message.lock);
        }
    }
}

// ===== Public API =====

void telemetry_report_event(Telemetry* instance, TelemetryEventType type, cJSON* data) {
    furi_check(instance);
    furi_check(type < TelemetryEventMax);

    TelemetryApiMessage message = {
        .type = TelemetryApiMessageTypeReportEvent,
        .data.report_event =
            {
                .type = type,
                .data = data,
            },
    };

    const FuriStatus status = furi_message_queue_put(instance->api_queue, &message, 0);
    if(status != FuriStatusOk) {
        FURI_LOG_W(TAG, "Failed to enqueue telemetry event %d", type);
        atomic_fetch_add(&instance->events_dropped, 1);
        if(data) {
            cJSON_Delete(data);
        }
    }
}

bool telemetry_is_enabled(Telemetry* instance) {
    furi_check(instance);
    return instance->is_enabled;
}

void telemetry_set_enabled(Telemetry* instance, bool enabled) {
    furi_check(instance);

    TelemetryApiMessage message = {
        .type = TelemetryApiMessageTypeSetEnabled,
        .data.is_enabled = enabled,
    };

    // Blocking: the opt-out change must always be applied.
    furi_check(
        furi_message_queue_put(instance->api_queue, &message, FuriWaitForever) == FuriStatusOk);
}

void telemetry_get_stats(Telemetry* instance, TelemetryStats* stats) {
    furi_check(instance);
    furi_check(stats);

    TelemetryApiMessage message = {
        .type = TelemetryApiMessageTypeGetStats,
        .data.get_stats =
            {
                .stats = stats,
            },
    };
    message.lock = api_lock_alloc_locked();

    furi_check(
        furi_message_queue_put(instance->api_queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message.lock);
}

// ===== Startup =====

static void telemetry_report_device_boot(Telemetry* instance) {
    cJSON* d = cJSON_CreateObject();

    const Version* version = version_get();
    if(version) {
        cJSON_AddStringToObject(
            d, "fw_version", version_get_version(version) ? version_get_version(version) : "");
        cJSON_AddStringToObject(
            d, "fw_hash", version_get_githash(version) ? version_get_githash(version) : "");
        cJSON_AddStringToObject(
            d, "fw_branch", version_get_gitbranch(version) ? version_get_gitbranch(version) : "");
        cJSON_AddStringToObject(
            d,
            "fw_build_date",
            version_get_builddate(version) ? version_get_builddate(version) : "");
        cJSON_AddNumberToObject(d, "fw_target", version_get_target(version));
        cJSON_AddBoolToObject(d, "fw_dirty", version_get_dirty_flag(version));
    }

    FuriString* serial = furi_string_alloc();
    hex_bytes_to_string(furi_hal_version_uid(), furi_hal_version_uid_size(), serial);
    cJSON_AddStringToObject(d, "serial", furi_string_get_cstr(serial));
    furi_string_free(serial);

    FuriString* mac = furi_string_alloc();
    hex_bytes_to_string(furi_hal_version_get_usb_mac(), 6, mac);
    cJSON_AddStringToObject(d, "usb_mac", furi_string_get_cstr(mac));
    furi_string_free(mac);

    cJSON_AddStringToObject(d, "hw_version", furi_hal_version_get_hw_version_code());

    telemetry_enqueue(instance, TelemetryEventDeviceBoot, d, true);
}

static void telemetry_detect_interrupted_timer(Telemetry* instance) {
    // A timer running at boot with an "unknown" source was restored from the
    // persisted snapshot — its original session was interrupted by reboot.
    BusyTimerRunInfo info;
    busy_timer_get_run_info(instance->busy_timer, &info);

    if(info.state != BusyTimerStateIdle && info.session_source == BusyTimerSessionSourceUnknown) {
        FURI_LOG_I(TAG, "Detected interrupted timer session at boot");

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "outcome", "interrupted");
        cJSON_AddStringToObject(d, "source", "restored");
        cJSON_AddNumberToObject(d, "duration_s", info.time_elapsed_s);
        cJSON_AddNumberToObject(d, "cycles", info.current_interval_idx);

        telemetry_enqueue(instance, TelemetryEventTimerSessionEnd, d, true);
    }
}

static Telemetry* telemetry_alloc(void) {
    Telemetry* instance = malloc(sizeof(Telemetry));

    TelemetrySettings settings;
    telemetry_settings_load(&settings);
    instance->is_enabled = settings.is_enabled;

    instance->event_loop = furi_event_loop_alloc();
    instance->api_queue =
        furi_message_queue_alloc(TELEMETRY_API_QUEUE_SIZE, sizeof(TelemetryApiMessage));

    instance->mqtt = furi_record_open(RECORD_MQTT);

    instance->flush_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        telemetry_flush_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    furi_event_loop_timer_start(
        instance->flush_timer, furi_ms_to_ticks(TELEMETRY_FLUSH_INTERVAL_MS));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        telemetry_message_queue_callback,
        instance);

    telemetry_collectors_init(instance);

    // Seed connectivity from the current MQTT status (pubsub delivers no initial value).
    const MqttStatus status = mqtt_get_status(instance->mqtt);
    instance->is_connected = (status == MqttStatusConnectedLinked) ||
                             (status == MqttStatusConnectedNotLinked);

    furi_record_create(RECORD_TELEMETRY, instance);

    telemetry_report_device_boot(instance);
    telemetry_detect_interrupted_timer(instance);

    return instance;
}

int32_t telemetry_srv(void* arg) {
    UNUSED(arg);

    Telemetry* instance = telemetry_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
