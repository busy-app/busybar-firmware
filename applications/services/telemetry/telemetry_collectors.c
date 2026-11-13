#include "telemetry_i.h"

#include <stdatomic.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <time/time.h>

#include <version.h>

static const char* telemetry_timer_type_to_string(BusyTimerMode mode) {
    switch(mode) {
    case BusyTimerModeInfinite:
        return "INFINITE";
    case BusyTimerModeSimple:
        return "SIMPLE";
    case BusyTimerModeInterval:
        return "INTERVAL";
    default:
        return "UNKNOWN";
    }
}

void telemetry_add_timer_config(cJSON* d, const BusyTimerConfig* config) {
    cJSON_AddStringToObject(d, "timer_type", telemetry_timer_type_to_string(config->mode));

    double total_ms = 0;
    switch(config->mode) {
    case BusyTimerModeSimple:
        total_ms = config->simple.total_time_ms;
        break;
    case BusyTimerModeInterval: {
        const BusyTimerIntervalConfig* interval = &config->interval;
        const double cycles = interval->cycles_count;
        total_ms = (double)interval->work_time_ms * cycles +
                   (cycles > 0 ? (double)interval->rest_time_ms * (cycles - 1) : 0);
        cJSON_AddNumberToObject(d, "timer_work_time_ms", interval->work_time_ms);
        cJSON_AddNumberToObject(d, "timer_rest_time_ms", interval->rest_time_ms);
        cJSON_AddNumberToObject(d, "timer_cycles_count", interval->cycles_count);
        cJSON_AddBoolToObject(d, "timer_is_autostart_enabled", interval->is_autostart_enabled);
        break;
    }
    default:
        break;
    }
    cJSON_AddNumberToObject(d, "timer_total_time_ms", total_ms);
}

void telemetry_generate_uuid(char out[37]) {
    uint8_t b[16];
    for(size_t i = 0; i < sizeof(b); i += 4) {
        const uint32_t r = furi_hal_random_get();
        memcpy(&b[i], &r, 4);
    }
    b[6] = (b[6] & 0x0F) | 0x40;
    b[8] = (b[8] & 0x3F) | 0x80;
    snprintf(
        out,
        37,
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        b[0],
        b[1],
        b[2],
        b[3],
        b[4],
        b[5],
        b[6],
        b[7],
        b[8],
        b[9],
        b[10],
        b[11],
        b[12],
        b[13],
        b[14],
        b[15]);
}

// Absolute path: APP_DATA_PATH resolves by the calling thread, and this file is
// touched from the busy_timer thread and the telemetry thread
#define TELEMETRY_TIMER_SESSION_PATH EXT_PATH("apps_data/telemetry/timer_session.txt")

void telemetry_timer_session_persist(const char* session_id, const char* source, uint32_t total_ms) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, EXT_PATH("apps_data/telemetry"));
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, TELEMETRY_TIMER_SESSION_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char line[64];
        snprintf(line, sizeof(line), "%s\n%s\n%lu", session_id, source, (unsigned long)total_ms);
        storage_file_write(file, line, strlen(line));
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool telemetry_timer_session_load(
    char session_id[37],
    char* source,
    size_t source_size,
    uint32_t* total_ms) {
    char buf[37 + 1 + 32 + 1 + 12 + 1] = {0};
    bool loaded = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, TELEMETRY_TIMER_SESSION_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_read(file, buf, sizeof(buf) - 1);
        char* separator = strchr(buf, '\n');
        if(separator && (separator - buf) == 36) {
            *separator = '\0';
            strlcpy(session_id, buf, 37);
            char* total = strchr(separator + 1, '\n');
            if(total) {
                *total = '\0';
                *total_ms = strtoul(total + 1, NULL, 10);
            }
            strlcpy(source, separator + 1, source_size);
            loaded = true;
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return loaded;
}

void telemetry_timer_session_clear(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove(storage, TELEMETRY_TIMER_SESSION_PATH);
    furi_record_close(RECORD_STORAGE);
}

static const char* telemetry_profile_to_string(BusyTimerProfileId profile_id) {
    return profile_id < BusyTimerProfileIdMax ? busy_timer_get_profile_name(profile_id) : "none";
}

// timer_source in the app's vocabulary (LOCAL / WATCH / CLOUD / BAR) where the meaning matches
static const char* telemetry_timer_source_to_string(BusyTimerSessionSource source) {
    switch(source) {
    case BusyTimerSessionSourceDevice:
        return "BAR";
    case BusyTimerSessionSourceHttpApi:
        return "HTTP_API";
    case BusyTimerSessionSourceIntegrationMatter:
        return "MATTER";
    case BusyTimerSessionSourceIntegrationMqtt:
        return "CLOUD";
    default:
        return "UNKNOWN";
    }
}

// reason in the app's vocabulary (USER_ACTION / EXTERNAL), plus INTERRUPTED
static const char* telemetry_timer_stop_reason_to_string(BusyTimerSessionOutcome outcome) {
    switch(outcome) {
    case BusyTimerSessionOutcomeStopped:
        return "USER_ACTION";
    case BusyTimerSessionOutcomeStoppedExternal:
        return "EXTERNAL";
    case BusyTimerSessionOutcomeInterrupted:
        return "INTERRUPTED";
    default:
        return "UNKNOWN";
    }
}

static const char* telemetry_switch_position_to_string(InputKey key) {
    switch(key) {
    case InputKeyBusy:
        return "busy";
    case InputKeyCustom:
        return "custom";
    case InputKeyOff:
        return "off";
    case InputKeyApps:
        return "apps";
    case InputKeySettings:
        return "settings";
    default:
        return "unknown";
    }
}

static const char* telemetry_wifi_security_to_string(WifiSecurityMode mode) {
    switch(mode) {
    case WifiSecurityModeOpen:
        return "open";
    case WifiSecurityModeWpa:
        return "wpa";
    case WifiSecurityModeWpa2:
        return "wpa2";
    case WifiSecurityModeWep:
        return "wep";
    case WifiSecurityModeWpaWpa2Mixed:
        return "wpa_wpa2_mixed";
    case WifiSecurityModeWpa3:
        return "wpa3";
    case WifiSecurityModeWpa3Transition:
        return "wpa3_transition";
    default:
        return "unknown";
    }
}

static void telemetry_mqtt_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const MqttEvent* event = message;

    switch(event->type) {
    case MqttEventTypeStatusChanged: {
        TelemetryApiMessage api_message = {
            .type = TelemetryApiMessageTypeMqttStatus,
            .data.mqtt_status = event->status_changed.status,
        };
        furi_message_queue_put(instance->api_queue, &api_message, 0);
        break;
    }
    case MqttEventTypeLinkDone: {
        cJSON* d = cJSON_CreateObject();
        cJSON_AddBoolToObject(d, "linked", true);
        telemetry_report_event(instance, TelemetryEventAccountLink, d);
        break;
    }
    case MqttEventTypeUnlinked: {
        cJSON* d = cJSON_CreateObject();
        cJSON_AddBoolToObject(d, "linked", false);
        telemetry_report_event(instance, TelemetryEventAccountUnlink, d);
        break;
    }
    default:
        break;
    }
}

static void telemetry_loader_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const LoaderEvent* event = message;

    switch(event->type) {
    case LoaderEventTypeApplicationBeforeLoad:
    case LoaderEventTypeApplicationStopped: {
        if(!event->appid) {
            return;
        }

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "app", event->appid);

        const TelemetryEventType type = (event->type == LoaderEventTypeApplicationBeforeLoad) ?
                                            TelemetryEventAppStart :
                                            TelemetryEventAppStop;
        telemetry_report_event(instance, type, d);
        break;
    }
    default:
        break;
    }
}

static void telemetry_busy_timer_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const BusyTimerEvent* event = message;

    switch(event->type) {
    case BusyTimerEventTypeStateChanged:
        if(event->state_changed.state == BusyTimerStateWork ||
           event->state_changed.state == BusyTimerStateRest) {
            instance->timer_last_active_state = event->state_changed.state;
        }
        break;

    case BusyTimerEventTypeSessionStarted: {
        const BusyTimerEventSessionStarted* started = &event->session_started;

        if(started->source == BusyTimerSessionSourceUnknown) {
            // Restored from the saved state at boot: reported on the telemetry thread
            TelemetryApiMessage api_message = {
                .type = TelemetryApiMessageTypeTimerRestored,
            };
            furi_message_queue_put(instance->api_queue, &api_message, 0);
            break;
        }

        instance->timer_session_active = true;
        instance->timer_session_start_ms = time_get_timestamp_ms();
        telemetry_generate_uuid(instance->timer_session_id);
        strlcpy(instance->timer_card_id, started->card_id, sizeof(instance->timer_card_id));
        instance->timer_session_config = started->timer_config;

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "uuid", instance->timer_card_id);
        cJSON_AddStringToObject(d, "timer_session_id", instance->timer_session_id);
        cJSON_AddStringToObject(
            d, "timer_source", telemetry_timer_source_to_string(started->source));
        cJSON_AddStringToObject(d, "profile", telemetry_profile_to_string(started->profile_id));
        cJSON_AddStringToObject(d, "theme", started->app_config.theme_name);
        telemetry_add_timer_config(d, &started->timer_config);
        cJSON_AddBoolToObject(d, "demo", started->is_demo_mode_enabled);
        cJSON_AddBoolToObject(d, "restored", started->source == BusyTimerSessionSourceUnknown);

        const uint32_t total_ms = started->timer_config.mode == BusyTimerModeSimple ?
                                      started->timer_config.simple.total_time_ms :
                                      0;
        telemetry_timer_session_persist(
            instance->timer_session_id,
            telemetry_timer_source_to_string(started->source),
            total_ms);
        telemetry_report_event(instance, TelemetryEventTimerStart, d);
        break;
    }
    case BusyTimerEventTypeSessionEnded: {
        const BusyTimerEventSessionEnded* ended = &event->session_ended;

        if(!instance->timer_session_active) {
            // Started before this service subscribed; no context to pair with
            telemetry_generate_uuid(instance->timer_session_id);
        }

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "uuid", instance->timer_card_id);
        cJSON_AddStringToObject(d, "timer_session_id", instance->timer_session_id);
        telemetry_add_timer_config(d, &instance->timer_session_config);
        cJSON_AddStringToObject(
            d, "timer_source", telemetry_timer_source_to_string(ended->source));
        uint32_t duration_s = ended->time_elapsed_s;
        if(instance->timer_session_config.mode == BusyTimerModeInfinite &&
           instance->timer_session_active) {
            // An infinite timer does not tick, so busy_timer does not count its time
            duration_s = (time_get_timestamp_ms() - instance->timer_session_start_ms) / 1000;
        }
        cJSON_AddNumberToObject(d, "duration_s", duration_s);
        cJSON_AddNumberToObject(d, "cycles", ended->cycles_completed);
        if(instance->timer_session_config.mode == BusyTimerModeInterval) {
            cJSON_AddStringToObject(
                d, "phase", ended->state == BusyTimerStateRest ? "rest" : "work");
        }

        TelemetryEventType type = TelemetryEventTimerComplete;
        if(ended->outcome != BusyTimerSessionOutcomeCompleted) {
            type = TelemetryEventTimerStop;
            cJSON_AddStringToObject(
                d, "reason", telemetry_timer_stop_reason_to_string(ended->outcome));
        }

        instance->timer_session_active = false;
        telemetry_timer_session_clear();
        telemetry_report_event(instance, type, d);
        break;
    }
    case BusyTimerEventTypeProfileSaved: {
        const BusyTimerEventProfileSaved* saved = &event->profile_saved;

        // A profile saved in the app or pushed from the cloud is reported by the app
        if(saved->source == BusyTimerSessionSourceIntegrationMqtt) {
            break;
        }

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "uuid", saved->profile.metadata.card_id);
        cJSON_AddStringToObject(d, "profile", telemetry_profile_to_string(saved->profile_id));
        cJSON_AddStringToObject(d, "theme", saved->profile.app_config.theme_name);
        telemetry_add_timer_config(d, &saved->profile.timer_config);
        cJSON_AddStringToObject(
            d, "timer_source", telemetry_timer_source_to_string(saved->source));

        telemetry_report_event(instance, TelemetryEventChangeTimerSettings, d);
        break;
    }
    default:
        break;
    }
}

static void telemetry_power_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const PowerEvent* event = message;
    if(event->type != PowerEventUsbConnectionStateUpdate) {
        return;
    }

    // Dispatch because power_get_info() cannot be called from the power task.
    TelemetryApiMessage api_message = {
        .type = TelemetryApiMessageTypePowerEvent,
    };
    furi_message_queue_put(instance->api_queue, &api_message, 0);
}

static void telemetry_audio_pubsub_callback(const void* message, void* context) {
    Telemetry* instance = context;
    const AudioEvent* event = message;

    if(event->type == AudioEventVolumeUpdate) {
        // Dispatch because audio_get_volume() cannot be called from the audio task.
        TelemetryApiMessage api_message = {
            .type = TelemetryApiMessageTypeAudioEvent,
        };
        furi_message_queue_put(instance->api_queue, &api_message, 0);
    }
}

static void telemetry_brightness_state_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const BrightnessControlState* state = item;

    // Report the user's setting, not light-sensor driven changes of the effective value
    if(state->brightness_setting == instance->last_brightness_value &&
       state->mode == instance->last_brightness_mode) {
        return;
    }
    instance->last_brightness_value = state->brightness_setting;
    instance->last_brightness_mode = state->mode;

    cJSON* d = cJSON_CreateObject();
    cJSON_AddNumberToObject(d, "value", state->brightness_setting);
    cJSON_AddStringToObject(
        d, "mode", state->mode == BrightnessControlBrightnessModeAuto ? "auto" : "manual");

    telemetry_report_event(instance, TelemetryEventSettingBrightness, d);
}

static void telemetry_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const InputEvent* event = message;

    if(event->type != InputTypePress) {
        return;
    }

    // p0 input: skip while disabled or offline, as telemetry_enqueue() would drop it
    if(!instance->is_enabled || !instance->is_connected) {
        return;
    }

    switch(event->key) {
    case InputKeyOk:
        atomic_fetch_add(&instance->input_ok, 1);
        break;
    case InputKeyBack:
        atomic_fetch_add(&instance->input_back, 1);
        break;
    case InputKeyStart:
        atomic_fetch_add(&instance->input_start, 1);
        break;
    case InputKeyUp:
        atomic_fetch_add(&instance->input_wheel_up, 1);
        break;
    case InputKeyDown:
        atomic_fetch_add(&instance->input_wheel_down, 1);
        break;
    case InputKeyBusy:
    case InputKeyCustom:
    case InputKeyOff:
    case InputKeyApps:
    case InputKeySettings: {
        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "pos", telemetry_switch_position_to_string(event->key));
        telemetry_report_event(instance, TelemetryEventInputSwitch, d);
        break;
    }
    default:
        break;
    }
}

static void telemetry_updater_state_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const UpdaterUpdateState* state = item;
    const Version* version = version_get();
    const char* from_version = version ? version_get_version(version) : "";

    if(state->event == UpdaterUpdateEventActionDone && state->status != UpdaterStatusOk &&
       state->status != UpdaterStatusDownloadAbort && state->status != UpdaterStatusBusy) {
        // Failed before the reboot (download, checksum, unpack, prepare)
        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "from_version", from_version);
        cJSON_AddStringToObject(d, "outcome", "failure");
        telemetry_report_event(instance, TelemetryEventFwUpdate, d);

    } else if(
        state->event == UpdaterUpdateEventActionBegin &&
        state->action == UpdaterUpdateActionInstallationApply) {
        // The result is known only after the reboot: remember where we came from
        telemetry_fw_update_marker_save(from_version);
    }
}

static void telemetry_canvas_ownership_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const CanvasOwnershipInfo* info = item;

    if(info->is_active) {
        if(!instance->last_canvas_active ||
           (strcmp(instance->last_canvas_app, info->app_id) != 0)) {
            instance->last_canvas_active = true;
            strlcpy(instance->last_canvas_app, info->app_id, sizeof(instance->last_canvas_app));

            cJSON* d = cJSON_CreateObject();
            cJSON_AddStringToObject(d, "app", info->app_id);
            cJSON_AddNumberToObject(d, "priority", info->priority);
            telemetry_report_event(instance, TelemetryEventCanvasAcquire, d);
        }
    } else {
        if(instance->last_canvas_active) {
            cJSON* d = cJSON_CreateObject();
            cJSON_AddStringToObject(d, "app", instance->last_canvas_app);
            instance->last_canvas_active = false;
            instance->last_canvas_app[0] = '\0';
            telemetry_report_event(instance, TelemetryEventCanvasRelease, d);
        }
    }
}

static void telemetry_wifi_state_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const WifiInfo* info = item;
    const WifiState state = info->state;

    if(state == instance->last_wifi_state) {
        return;
    }

    const WifiState prev_state = instance->last_wifi_state;
    instance->last_wifi_state = state;

    if(state == WifiStateConnected) {
        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(
            d, "security", telemetry_wifi_security_to_string(info->security_mode));
        cJSON_AddBoolToObject(d, "reconnect", prev_state == WifiStateReconnecting);
        telemetry_report_event(instance, TelemetryEventNetWifiConnect, d);

    } else if(state == WifiStateDisconnected) {
        const char* reason = "unknown";
        if(prev_state == WifiStateDisconnecting) {
            reason = "requested";
        } else if(prev_state == WifiStateConnecting) {
            reason = "connect_failed";
        } else if(prev_state == WifiStateReconnecting) {
            reason = "connection_lost";
        }

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "reason", reason);
        telemetry_report_event(instance, TelemetryEventNetWifiDisconnect, d);
    }
}

static void telemetry_wifi_action_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const WifiAction action = *(const WifiAction*)message;

    cJSON* d = cJSON_CreateObject();
    cJSON_AddStringToObject(d, "action", action == WifiActionConnect ? "connect" : "forget");
    telemetry_report_event(instance, TelemetryEventNetWifiReconfigure, d);
}

static void telemetry_matter_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const MatterEvent* event = message;
    if(event->type != MatterEventTypeFabricCountChanged) {
        return;
    }

    const size_t count = event->fabric_count;
    const size_t last = event->previous_fabric_count;
    instance->last_matter_fabrics = count;

    if(count == last) {
        return;
    }

    cJSON* d = cJSON_CreateObject();
    cJSON_AddStringToObject(d, "integration", "matter");
    telemetry_report_event(
        instance,
        count > last ? TelemetryEventIntegrationConnect : TelemetryEventIntegrationDisconnect,
        d);
}

void telemetry_collectors_init(Telemetry* instance) {
    instance->mqtt_pubsub = mqtt_get_pubsub(instance->mqtt);
    furi_pubsub_subscribe(instance->mqtt_pubsub, telemetry_mqtt_pubsub_callback, instance);

    instance->loader = furi_record_open(RECORD_LOADER);
    instance->loader_pubsub = loader_get_pubsub(instance->loader);
    furi_pubsub_subscribe(instance->loader_pubsub, telemetry_loader_pubsub_callback, instance);

    instance->busy_timer = furi_record_open(RECORD_BUSY_TIMER);
    instance->busy_timer_pubsub = busy_timer_get_pubsub(instance->busy_timer);
    furi_pubsub_subscribe(
        instance->busy_timer_pubsub, telemetry_busy_timer_pubsub_callback, instance);

    instance->power = furi_record_open(RECORD_POWER);
    instance->power_pubsub = power_get_pubsub(instance->power);
    furi_pubsub_subscribe(instance->power_pubsub, telemetry_power_pubsub_callback, instance);

    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->audio_pubsub = audio_get_pubsub(instance->audio);
    furi_pubsub_subscribe(instance->audio_pubsub, telemetry_audio_pubsub_callback, instance);

    // get_subscribe: no initial notification; seed the dedup value
    instance->brightness_control = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    instance->brightness_state = brightness_control_get_state(instance->brightness_control);
    BrightnessControlState brightness_state;
    furi_state_get_subscribe(
        instance->brightness_state,
        &brightness_state,
        telemetry_brightness_state_callback,
        instance);
    instance->last_brightness_value = brightness_state.brightness_setting;
    instance->last_brightness_mode = brightness_state.mode;

    instance->input_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(instance->input_pubsub, telemetry_input_pubsub_callback, instance);

    instance->updater = furi_record_open(RECORD_UPDATER);
    instance->updater_state = updater_get_update_state(instance->updater);
    furi_state_subscribe(instance->updater_state, telemetry_updater_state_callback, instance);

    instance->matter = furi_record_open(RECORD_MATTER);
    MatterCommissionedFabrics fabrics;
    instance->last_matter_fabrics = matter_get_commissioned_fabrics(instance->matter, &fabrics) ==
                                            MatterStatusOk ?
                                        fabrics.count :
                                        0;
    instance->matter_pubsub = matter_get_pubsub(instance->matter);
    furi_pubsub_subscribe(instance->matter_pubsub, telemetry_matter_pubsub_callback, instance);

    // get_subscribe: no initial notification; seed the dedup value
    CanvasSrv* canvas = furi_record_open(RECORD_CANVAS);
    instance->canvas_ownership_state = canvas_get_ownership_state(canvas);
    CanvasOwnershipInfo ownership_info;
    furi_state_get_subscribe(
        instance->canvas_ownership_state,
        &ownership_info,
        telemetry_canvas_ownership_callback,
        instance);
    instance->last_canvas_active = ownership_info.is_active;
    strlcpy(instance->last_canvas_app, ownership_info.app_id, sizeof(instance->last_canvas_app));

    // a draw may predate this service
    if(ownership_info.is_active) {
        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "app", ownership_info.app_id);
        cJSON_AddNumberToObject(d, "priority", ownership_info.priority);
        telemetry_report_event(instance, TelemetryEventCanvasAcquire, d);
    }

    // get_subscribe: no initial notification; seed the dedup value
    instance->wifi = furi_record_open(RECORD_WIFI);
    instance->wifi_state = wifi_get_state(instance->wifi);
    WifiInfo wifi_info;
    furi_state_get_subscribe(
        instance->wifi_state, &wifi_info, telemetry_wifi_state_callback, instance);
    instance->last_wifi_state = wifi_info.state;
    instance->wifi_action_pubsub = wifi_get_action_pubsub(instance->wifi);
    furi_pubsub_subscribe(instance->wifi_action_pubsub, telemetry_wifi_action_callback, instance);
}
