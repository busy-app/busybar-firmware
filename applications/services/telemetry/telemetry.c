#include "telemetry_i.h"
#include "telemetry_settings.h"

#include <storage/storage.h>
#include <toolbox/update_lib/common_vals.h>

#include <time/time.h>
#include <version.h>
#include <furi_hal_version.h>
#include <furi_hal_nvm.h>
#include <toolbox/hex.h>

#include <power/power_service/power.h>
#include <audio/audio.h>
#include <matter/matter.h>

typedef struct {
    const char* name;
    TelemetryPriority priority;
} TelemetryEventInfo;

static const TelemetryEventInfo telemetry_event_info[TelemetryEventMax] = {
    [TelemetryEventDeviceBoot] = {"device.boot", TelemetryPriorityPush},
    [TelemetryEventFwUpdate] = {"fw.update", TelemetryPriorityPush},
    [TelemetryEventTimerStart] = {"start_timer", TelemetryPriorityBatch},
    [TelemetryEventTimerComplete] = {"timer_complete", TelemetryPriorityPush},
    [TelemetryEventTimerStop] = {"timer_stop", TelemetryPriorityPush},
    [TelemetryEventChangeTimerSettings] = {"change_timer_settings", TelemetryPriorityLow},
    [TelemetryEventAppStart] = {"app.start", TelemetryPriorityBatch},
    [TelemetryEventAppStop] = {"app.stop", TelemetryPriorityBatch},
    [TelemetryEventAppInstall] = {"app.install", TelemetryPriorityBatch},
    [TelemetryEventAppUpdate] = {"app.update", TelemetryPriorityBatch},
    [TelemetryEventAppUninstall] = {"app.uninstall", TelemetryPriorityBatch},
    [TelemetryEventSettingBrightness] = {"setting.brightness", TelemetryPriorityLow},
    [TelemetryEventSettingVolume] = {"setting.volume", TelemetryPriorityLow},
    [TelemetryEventInputSwitch] = {"input.switch", TelemetryPriorityLow},
    [TelemetryEventPowerTransition] = {"power.transition", TelemetryPriorityPush},
    [TelemetryEventNetOnline] = {"net.online", TelemetryPriorityPush},
    [TelemetryEventNetOffline] = {"net.offline", TelemetryPriorityPush},
    [TelemetryEventNetOfflineDuration] = {"net.offline_duration", TelemetryPriorityPush},
    [TelemetryEventNetWifiConnect] = {"net.wifi.connect", TelemetryPriorityPush},
    [TelemetryEventNetWifiDisconnect] = {"net.wifi.disconnect", TelemetryPriorityPush},
    [TelemetryEventNetWifiReconfigure] = {"net.wifi.reconfigure", TelemetryPriorityPush},
    [TelemetryEventAccountLink] = {"account.link", TelemetryPriorityPush},
    [TelemetryEventAccountUnlink] = {"account.unlink", TelemetryPriorityPush},
    [TelemetryEventCanvasAcquire] = {"canvas.acquire", TelemetryPriorityBatch},
    [TelemetryEventCanvasRelease] = {"canvas.release", TelemetryPriorityBatch},
    [TelemetryEventIntegrationConnect] = {"integration.connect", TelemetryPriorityPush},
    [TelemetryEventIntegrationDisconnect] = {"integration.disconnect", TelemetryPriorityPush},
    [TelemetryEventTelemetryEnable] = {"telemetry.enable", TelemetryPriorityPush},
    [TelemetryEventTelemetryDisable] = {"telemetry.disable", TelemetryPriorityPush},
};

static_assert(COUNT_OF(telemetry_event_info) == TelemetryEventMax);

const char* telemetry_event_type_name(TelemetryEventType type) {
    furi_check(type < TelemetryEventMax);
    return telemetry_event_info[type].name;
}

static cJSON* telemetry_ring_pop(Telemetry* instance) {
    furi_assert(instance->events_count > 0);

    cJSON* event = instance->events[instance->events_head];
    instance->events_head = (instance->events_head + 1) % TELEMETRY_RING_CAPACITY;
    instance->events_count--;
    return event;
}

static void telemetry_ring_push(Telemetry* instance, cJSON* event) {
    if(instance->events_count >= TELEMETRY_RING_CAPACITY) {
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

static void telemetry_add_device_state(Telemetry* instance, cJSON* events) {
    cJSON* d = cJSON_CreateObject();

    PowerInfo power_info;
    power_get_info(instance->power, &power_info);
    cJSON_AddNumberToObject(d, "charge", power_info.charge);
    cJSON_AddBoolToObject(d, "charging", power_info.is_charging);
    cJSON_AddBoolToObject(d, "usb_connected", power_is_usb_connected(instance->power));
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

typedef struct {
    uint32_t ok;
    uint32_t back;
    uint32_t start;
    uint32_t wheel_up;
    uint32_t wheel_down;
} TelemetryInputCounts;

static void telemetry_input_counts_take(Telemetry* instance, TelemetryInputCounts* counts) {
    counts->ok = atomic_exchange(&instance->input_ok, 0);
    counts->back = atomic_exchange(&instance->input_back, 0);
    counts->start = atomic_exchange(&instance->input_start, 0);
    counts->wheel_up = atomic_exchange(&instance->input_wheel_up, 0);
    counts->wheel_down = atomic_exchange(&instance->input_wheel_down, 0);
}

static void
    telemetry_input_counts_restore(Telemetry* instance, const TelemetryInputCounts* counts) {
    atomic_fetch_add(&instance->input_ok, counts->ok);
    atomic_fetch_add(&instance->input_back, counts->back);
    atomic_fetch_add(&instance->input_start, counts->start);
    atomic_fetch_add(&instance->input_wheel_up, counts->wheel_up);
    atomic_fetch_add(&instance->input_wheel_down, counts->wheel_down);
}

static void telemetry_input_counts_clear(Telemetry* instance) {
    atomic_store(&instance->input_ok, 0);
    atomic_store(&instance->input_back, 0);
    atomic_store(&instance->input_start, 0);
    atomic_store(&instance->input_wheel_up, 0);
    atomic_store(&instance->input_wheel_down, 0);
}

static void
    telemetry_add_input_counts(Telemetry* instance, cJSON* events, TelemetryInputCounts* counts) {
    telemetry_input_counts_take(instance, counts);

    if(counts->ok == 0 && counts->back == 0 && counts->start == 0 && counts->wheel_up == 0 &&
       counts->wheel_down == 0) {
        return;
    }

    cJSON* d = cJSON_CreateObject();
    cJSON_AddNumberToObject(d, "ok", counts->ok);
    cJSON_AddNumberToObject(d, "back", counts->back);
    cJSON_AddNumberToObject(d, "start", counts->start);
    cJSON_AddNumberToObject(d, "wheel_up", counts->wheel_up);
    cJSON_AddNumberToObject(d, "wheel_down", counts->wheel_down);

    cJSON* event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "t", "input.counts");
    cJSON_AddNumberToObject(event, "ts", (double)time_get_timestamp_ms());
    cJSON_AddNumberToObject(event, "p", TelemetryPriorityLow);
    cJSON_AddItemToObject(event, "d", d);
    cJSON_AddItemToArray(events, event);
}

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

    // The ring keeps ownership of the buffered events until the batch is published
    for(size_t i = 0; i < instance->events_count; i++) {
        const size_t index = (instance->events_head + i) % TELEMETRY_RING_CAPACITY;
        cJSON_AddItemReferenceToArray(events, instance->events[index]);
    }

    TelemetryInputCounts input_counts = {0};
    if(!is_push) {
        telemetry_add_device_state(instance, events);
        telemetry_add_input_counts(instance, events, &input_counts);
    }

    if(cJSON_GetArraySize(events) == 0) {
        cJSON_Delete(batch);
        return;
    }

    char* json = cJSON_PrintUnformatted(batch);
    bool published = false;
    if(json) {
        FURI_LOG_I(
            TAG,
            "Publishing %u telemetry events (%zu bytes)",
            cJSON_GetArraySize(events),
            strlen(json));
        published = mqtt_publish_device_scope(
            instance->mqtt, TELEMETRY_MQTT_QOS, TELEMETRY_MQTT_TOPIC, json, strlen(json));
        free(json);
    }

    if(published) {
        while(instance->events_count > 0) {
            cJSON_Delete(telemetry_ring_pop(instance));
        }
        instance->batches_sent++;
        instance->events_sent += cJSON_GetArraySize(events);
    } else {
        FURI_LOG_W(TAG, "Failed to publish telemetry batch");
        telemetry_input_counts_restore(instance, &input_counts);
    }
    cJSON_Delete(batch);
}

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

static void
    telemetry_handle_report_event(Telemetry* instance, const TelemetryApiMessage* message) {
    const TelemetryApiMessageReportEvent* report = &message->data.report_event;
    furi_assert(report->type < TelemetryEventMax);

    telemetry_enqueue(instance, report->type, report->data, true);
}

static void telemetry_save_settings(Telemetry* instance);
static bool telemetry_publish_disable_now(Telemetry* instance);

static void telemetry_handle_mqtt_status(Telemetry* instance, MqttStatus status) {
    const bool connected = (status == MqttStatusConnectedLinked) ||
                           (status == MqttStatusConnectedNotLinked);
    const bool was_connected = instance->is_connected;
    instance->is_connected = connected;

    if(connected && !was_connected) {
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

        if(!instance->is_enabled && instance->is_disable_pending &&
           telemetry_publish_disable_now(instance)) {
            instance->is_disable_pending = false;
            telemetry_save_settings(instance);
        }

    } else if(!connected && was_connected) {
        instance->offline_start_ms = time_get_timestamp_ms();
        instance->has_offline_start = true;
        telemetry_enqueue(instance, TelemetryEventNetOffline, NULL, true);
    }
}

static void telemetry_handle_power_event(Telemetry* instance) {
    PowerInfo info;
    power_get_info(instance->power, &info);

    cJSON* d = cJSON_CreateObject();
    cJSON_AddBoolToObject(d, "usb_connected", power_is_usb_connected(instance->power));
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

static void telemetry_save_settings(Telemetry* instance) {
    TelemetrySettings settings = {
        .is_enabled = instance->is_enabled,
        .is_disable_pending = instance->is_disable_pending,
    };
    telemetry_settings_save(&settings);
}

// Publishes telemetry.disable on its own, regardless of the consent state
static bool telemetry_publish_disable_now(Telemetry* instance) {
    if(!instance->is_connected) {
        return false;
    }

    cJSON* batch = cJSON_CreateObject();
    cJSON_AddNumberToObject(batch, "schema", TELEMETRY_SCHEMA);
    cJSON_AddNumberToObject(batch, "ts", (double)time_get_timestamp_ms());
    cJSON* events = cJSON_AddArrayToObject(batch, "events");
    cJSON_AddItemToArray(events, telemetry_event_create(TelemetryEventTelemetryDisable, NULL));

    char* json = cJSON_PrintUnformatted(batch);
    bool published = false;
    if(json) {
        published = mqtt_publish_device_scope(
            instance->mqtt, TELEMETRY_MQTT_QOS, TELEMETRY_MQTT_TOPIC, json, strlen(json));
        free(json);
    }
    cJSON_Delete(batch);

    if(published) {
        instance->batches_sent++;
        instance->events_sent++;
    }
    return published;
}

static void telemetry_handle_set_enabled(
    Telemetry* instance,
    const TelemetryApiMessageSetEnabled* request) {
    const bool enabled = request->is_enabled;
    if(instance->is_enabled == enabled) {
        return;
    }

    if(!enabled) {
        // Send what is buffered, then telemetry.disable as the last event. Without a
        // connection the buffer is dropped and telemetry.disable waits for the next one.
        instance->last_push_ms = 0;
        telemetry_flush(instance, true);
        instance->is_disable_pending = !telemetry_publish_disable_now(instance);
    } else if(instance->is_disable_pending) {
        // Consent given again before the revocation was delivered: deliver it first
        telemetry_publish_disable_now(instance);
        instance->is_disable_pending = false;
    }

    instance->is_enabled = enabled;
    telemetry_save_settings(instance);

    if(!enabled) {
        telemetry_ring_clear(instance);
        telemetry_input_counts_clear(instance);
    } else {
        cJSON* d = cJSON_CreateObject();
        if(request->source[0]) {
            cJSON_AddStringToObject(d, "source", request->source);
        }
        if(request->text_version[0]) {
            cJSON_AddStringToObject(d, "text_version", request->text_version);
        }
        telemetry_enqueue(instance, TelemetryEventTelemetryEnable, d, true);
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

static void telemetry_flush_timer_callback(void* context) {
    furi_assert(context);
    Telemetry* instance = context;

    telemetry_flush(instance, false);
}

static void telemetry_detect_interrupted_timer(Telemetry* instance);

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
            telemetry_handle_set_enabled(instance, &message.data.set_enabled);
            break;
        case TelemetryApiMessageTypeGetStats:
            telemetry_handle_get_stats(instance, &message);
            break;
        case TelemetryApiMessageTypeFlush:
            telemetry_flush(instance, false);
            break;
        case TelemetryApiMessageTypeTimerRestored:
            telemetry_detect_interrupted_timer(instance);
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
    telemetry_set_enabled_with_source(instance, enabled, "device", NULL);
}

void telemetry_set_enabled_with_source(
    Telemetry* instance,
    bool enabled,
    const char* source,
    const char* text_version) {
    furi_check(instance);

    TelemetryApiMessage message = {
        .type = TelemetryApiMessageTypeSetEnabled,
        .data.set_enabled.is_enabled = enabled,
    };
    if(source) {
        strlcpy(message.data.set_enabled.source, source, sizeof(message.data.set_enabled.source));
    }
    if(text_version) {
        strlcpy(
            message.data.set_enabled.text_version,
            text_version,
            sizeof(message.data.set_enabled.text_version));
    }
    message.lock = api_lock_alloc_locked();

    furi_check(
        furi_message_queue_put(instance->api_queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message.lock);
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

// Absolute path: written from the updater thread, read on the telemetry thread
#define TELEMETRY_FW_UPDATE_MARKER_PATH  EXT_PATH("apps_data/telemetry/fw_update_pending.txt")
#define TELEMETRY_FW_UPDATE_SESSION_PATH EXT_PATH(SESSION_CONFIG_FILE_NAME)

void telemetry_fw_update_marker_save(const char* from_version) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, EXT_PATH("apps_data/telemetry"));
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, TELEMETRY_FW_UPDATE_MARKER_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_write(file, from_version, strlen(from_version));
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void telemetry_report_fw_update_result(Telemetry* instance) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    char from_version[64] = {0};
    bool has_marker = false;
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, TELEMETRY_FW_UPDATE_MARKER_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_read(file, from_version, sizeof(from_version) - 1);
        has_marker = true;
    }
    storage_file_close(file);
    storage_file_free(file);

    if(has_marker) {
        storage_simply_remove(storage, TELEMETRY_FW_UPDATE_MARKER_PATH);

        // The update executor deletes its session config only when the update succeeded
        const bool failed = storage_file_exists(storage, TELEMETRY_FW_UPDATE_SESSION_PATH);

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "from_version", from_version);
        if(!failed) {
            const Version* version = version_get();
            cJSON_AddStringToObject(d, "to_version", version ? version_get_version(version) : "");
        }
        cJSON_AddStringToObject(d, "outcome", failed ? "failure" : "success");
        telemetry_enqueue(instance, TelemetryEventFwUpdate, d, true);
    }

    furi_record_close(RECORD_STORAGE);
}

static void telemetry_detect_interrupted_timer(Telemetry* instance) {
    // A timer running at boot with an "unknown" source was restored from the
    // persisted snapshot: its original session was interrupted by the reboot,
    // and the restored one continues under a new session id.
    BusyTimerRunInfo info;
    busy_timer_get_run_info(instance->busy_timer, &info);

    // Runs at boot and again when busy_timer restores the session after this service
    // subscribed; whichever sees the restored session first reports it, once.
    if(instance->timer_restore_reported || info.state == BusyTimerStateIdle ||
       info.session_source != BusyTimerSessionSourceUnknown) {
        return;
    }
    instance->timer_restore_reported = true;

    FURI_LOG_I(TAG, "Detected interrupted timer session at boot");

    BusyTimerSnapshot snapshot;
    busy_timer_get_snapshot(instance->busy_timer, &snapshot);

    // Close the session that was running before the reboot under its own id, source and
    // planned duration (the saved snapshot carries only the time left)
    char interrupted_session_id[37];
    char interrupted_source[32] = "UNKNOWN";
    uint32_t interrupted_total_ms = 0;
    if(!telemetry_timer_session_load(
           interrupted_session_id,
           interrupted_source,
           sizeof(interrupted_source),
           &interrupted_total_ms)) {
        telemetry_generate_uuid(interrupted_session_id);
    }
    if(info.config.mode == BusyTimerModeSimple) {
        info.config.simple.total_time_ms =
            interrupted_total_ms ? interrupted_total_ms :
            snapshot.type == BusyTimerSnapshotTypeSimple ?
                                   info.time_elapsed_s * 1000 + snapshot.simple.time_left_ms :
                                   0;
    }

    cJSON* d = cJSON_CreateObject();
    cJSON_AddStringToObject(d, "uuid", snapshot.common.card_id);
    cJSON_AddStringToObject(d, "timer_session_id", interrupted_session_id);
    telemetry_add_timer_config(d, &info.config);
    cJSON_AddStringToObject(d, "reason", "INTERRUPTED");
    cJSON_AddStringToObject(d, "timer_source", interrupted_source);
    cJSON_AddNumberToObject(d, "duration_s", info.time_elapsed_s);
    cJSON_AddNumberToObject(d, "cycles", info.cycles_completed);
    if(info.config.mode == BusyTimerModeInterval) {
        cJSON_AddStringToObject(d, "phase", info.state == BusyTimerStateRest ? "rest" : "work");
    }
    telemetry_enqueue(instance, TelemetryEventTimerStop, d, true);

    instance->timer_session_active = true;
    telemetry_generate_uuid(instance->timer_session_id);
    strlcpy(instance->timer_card_id, snapshot.common.card_id, sizeof(instance->timer_card_id));
    instance->timer_session_config = info.config;
    instance->timer_last_active_state = info.state;
    instance->timer_session_start_ms = time_get_timestamp_ms();

    const char* profile_name = "none";
    bool is_demo = false;
    for(BusyTimerProfileId id = 0; id < BusyTimerProfileIdMax; ++id) {
        BusyTimerProfile profile;
        busy_timer_get_profile(instance->busy_timer, id, &profile);
        if(strcmp(profile.metadata.card_id, snapshot.common.card_id) == 0) {
            profile_name = busy_timer_get_profile_name(id);
            BusyTimerPreset preset;
            busy_timer_get_preset(instance->busy_timer, id, &preset);
            is_demo = preset.is_demo_mode_enabled;
            break;
        }
    }

    d = cJSON_CreateObject();
    cJSON_AddStringToObject(d, "uuid", instance->timer_card_id);
    cJSON_AddStringToObject(d, "timer_session_id", instance->timer_session_id);
    cJSON_AddStringToObject(d, "timer_source", "UNKNOWN");
    cJSON_AddStringToObject(d, "profile", profile_name);
    cJSON_AddStringToObject(d, "theme", snapshot.app_config.theme_name);
    telemetry_add_timer_config(d, &info.config);
    cJSON_AddBoolToObject(d, "demo", is_demo);
    cJSON_AddBoolToObject(d, "restored", true);
    telemetry_timer_session_persist(
        instance->timer_session_id,
        "UNKNOWN",
        info.config.mode == BusyTimerModeSimple ? info.config.simple.total_time_ms : 0);
    telemetry_enqueue(instance, TelemetryEventTimerStart, d, true);
}

static Telemetry* telemetry_alloc(void) {
    Telemetry* instance = malloc(sizeof(Telemetry));

    TelemetrySettings settings;
    telemetry_settings_load(&settings);
    instance->is_enabled = settings.is_enabled;
    instance->is_disable_pending = settings.is_disable_pending;

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
    telemetry_report_fw_update_result(instance);
    telemetry_detect_interrupted_timer(instance);

    return instance;
}

int32_t telemetry_srv(void* arg) {
    UNUSED(arg);

    Telemetry* instance = telemetry_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
