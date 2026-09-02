#include "telemetry_i.h"

#include <stdatomic.h>

#include <version.h>

static const char* telemetry_timer_mode_to_string(BusyTimerMode mode) {
    switch(mode) {
    case BusyTimerModeInfinite:
        return "infinite";
    case BusyTimerModeSimple:
        return "simple";
    case BusyTimerModeInterval:
        return "interval";
    default:
        return "unknown";
    }
}

static const char* telemetry_timer_source_to_string(BusyTimerSessionSource source) {
    switch(source) {
    case BusyTimerSessionSourceDevice:
        return "device";
    case BusyTimerSessionSourceHttpApi:
        return "http_api";
    case BusyTimerSessionSourceIntegrationMatter:
        return "integration:matter";
    case BusyTimerSessionSourceIntegrationMqtt:
        return "integration:mqtt";
    default:
        return "unknown";
    }
}

static const char* telemetry_timer_outcome_to_string(BusyTimerSessionOutcome outcome) {
    switch(outcome) {
    case BusyTimerSessionOutcomeCompleted:
        return "completed";
    case BusyTimerSessionOutcomeStopped:
        return "stopped";
    default:
        return "unknown";
    }
}

static const char* telemetry_switch_position_to_string(InputKey key) {
    switch(key) {
    case InputKeyBusy:
        return "busy";
    case InputKeyCustom:
        return "status";
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

// ===== MQTT =====

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

// ===== Loader =====

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

// ===== Busy timer =====

static void telemetry_busy_timer_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const BusyTimerEvent* event = message;

    switch(event->type) {
    case BusyTimerEventTypeSessionStarted: {
        const BusyTimerEventSessionStarted* started = &event->session_started;

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "source", telemetry_timer_source_to_string(started->source));
        if(started->profile_id < BusyTimerProfileIdMax) {
            cJSON_AddStringToObject(
                d, "profile", busy_timer_get_profile_name(started->profile_id));
        }
        cJSON_AddStringToObject(d, "theme", started->app_config.theme_name);
        cJSON_AddStringToObject(
            d, "mode", telemetry_timer_mode_to_string(started->timer_config.mode));
        cJSON_AddBoolToObject(d, "demo", started->is_demo_mode_enabled);

        switch(started->timer_config.mode) {
        case BusyTimerModeSimple:
            cJSON_AddNumberToObject(d, "duration_ms", started->timer_config.simple.total_time_ms);
            break;
        case BusyTimerModeInterval:
            cJSON_AddNumberToObject(d, "work_ms", started->timer_config.interval.work_time_ms);
            cJSON_AddNumberToObject(d, "rest_ms", started->timer_config.interval.rest_time_ms);
            cJSON_AddNumberToObject(d, "cycles", started->timer_config.interval.cycles_count);
            cJSON_AddBoolToObject(
                d, "autostart", started->timer_config.interval.is_autostart_enabled);
            break;
        default:
            break;
        }

        telemetry_report_event(instance, TelemetryEventTimerSessionStart, d);
        break;
    }
    case BusyTimerEventTypeSessionEnded: {
        const BusyTimerEventSessionEnded* ended = &event->session_ended;

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "outcome", telemetry_timer_outcome_to_string(ended->outcome));
        cJSON_AddStringToObject(d, "source", telemetry_timer_source_to_string(ended->source));
        cJSON_AddNumberToObject(d, "duration_s", ended->time_elapsed_s);
        cJSON_AddNumberToObject(d, "cycles", ended->current_interval_index);

        telemetry_report_event(instance, TelemetryEventTimerSessionEnd, d);
        break;
    }
    case BusyTimerEventTypeProfileChanged: {
        const BusyTimerEventProfileChanged* profile_changed = &event->profile_changed;

        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(
            d, "profile", busy_timer_get_profile_name(profile_changed->profile_id));
        cJSON_AddStringToObject(d, "theme", profile_changed->profile.app_config.theme_name);

        telemetry_report_event(instance, TelemetryEventTimerTheme, d);
        break;
    }
    default:
        break;
    }
}

// ===== Power =====

static void telemetry_power_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    Telemetry* instance = context;

    // Dispatch because power_get_info() cannot be called from the power task.
    TelemetryApiMessage api_message = {
        .type = TelemetryApiMessageTypePowerEvent,
    };
    furi_message_queue_put(instance->api_queue, &api_message, 0);
}

// ===== Audio =====

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

// ===== Brightness =====

static void telemetry_brightness_state_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const BrightnessControlState* state = item;

    if(state->effective_brightness == instance->last_brightness_value) {
        return;
    }
    instance->last_brightness_value = state->effective_brightness;

    cJSON* d = cJSON_CreateObject();
    cJSON_AddNumberToObject(d, "value", state->effective_brightness);
    cJSON_AddStringToObject(
        d, "mode", state->mode == BrightnessControlBrightnessModeAuto ? "auto" : "manual");

    telemetry_report_event(instance, TelemetryEventSettingBrightness, d);
}

// ===== Input =====

static void telemetry_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    Telemetry* instance = context;

    const InputEvent* event = message;

    if(event->type != InputTypePress) {
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

// ===== Updater =====

static void telemetry_updater_state_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const UpdaterUpdateState* state = item;

    if(state->event != UpdaterUpdateEventActionDone ||
       state->action != UpdaterUpdateActionInstallationApply) {
        return;
    }

    const Version* version = version_get();

    cJSON* d = cJSON_CreateObject();
    cJSON_AddStringToObject(d, "from_version", version ? version_get_version(version) : "");
    cJSON_AddStringToObject(
        d, "outcome", state->status == UpdaterStatusOk ? "success" : "failure");

    telemetry_report_event(instance, TelemetryEventFwUpdate, d);
}

// ===== Canvas ownership =====

static void telemetry_canvas_ownership_callback(const void* item, void* context) {
    Telemetry* instance = context;

    const CanvasOwnershipInfo* info = item;

    if(info->is_active) {
        // New or changed owner: emit canvas.acquire.
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
        // Released: emit canvas.release.
        if(instance->last_canvas_active) {
            cJSON* d = cJSON_CreateObject();
            cJSON_AddStringToObject(d, "app", instance->last_canvas_app);
            instance->last_canvas_active = false;
            instance->last_canvas_app[0] = '\0';
            telemetry_report_event(instance, TelemetryEventCanvasRelease, d);
        }
    }
}

// ===== Registration =====

void telemetry_collectors_init(Telemetry* instance) {
    // MQTT
    instance->mqtt_pubsub = mqtt_get_pubsub(instance->mqtt);
    furi_pubsub_subscribe(instance->mqtt_pubsub, telemetry_mqtt_pubsub_callback, instance);

    // Loader
    instance->loader = furi_record_open(RECORD_LOADER);
    instance->loader_pubsub = loader_get_pubsub(instance->loader);
    furi_pubsub_subscribe(instance->loader_pubsub, telemetry_loader_pubsub_callback, instance);

    // Busy timer
    instance->busy_timer = furi_record_open(RECORD_BUSY_TIMER);
    instance->busy_timer_pubsub = busy_timer_get_pubsub(instance->busy_timer);
    furi_pubsub_subscribe(
        instance->busy_timer_pubsub, telemetry_busy_timer_pubsub_callback, instance);

    // Power
    instance->power = furi_record_open(RECORD_POWER);
    instance->power_pubsub = power_get_pubsub(instance->power);
    furi_pubsub_subscribe(instance->power_pubsub, telemetry_power_pubsub_callback, instance);

    // Audio
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->audio_pubsub = audio_get_pubsub(instance->audio);
    furi_pubsub_subscribe(instance->audio_pubsub, telemetry_audio_pubsub_callback, instance);

    // Brightness (get_subscribe: no initial notification; seed the dedup value)
    instance->brightness_control = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    instance->brightness_state = brightness_control_get_state(instance->brightness_control);
    BrightnessControlState brightness_state;
    furi_state_get_subscribe(
        instance->brightness_state,
        &brightness_state,
        telemetry_brightness_state_callback,
        instance);
    instance->last_brightness_value = brightness_state.effective_brightness;

    // Input
    instance->input_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(instance->input_pubsub, telemetry_input_pubsub_callback, instance);

    // Updater
    instance->updater = furi_record_open(RECORD_UPDATER);
    instance->updater_state = updater_get_update_state(instance->updater);
    furi_state_subscribe(instance->updater_state, telemetry_updater_state_callback, instance);

    // Matter (used for the device.state snapshot)
    instance->matter = furi_record_open(RECORD_MATTER);

    // Canvas ownership (get_subscribe: no initial notification; seed the dedup value)
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

    // Report the current canvas owner once at startup (a draw may predate this service)
    if(ownership_info.is_active) {
        cJSON* d = cJSON_CreateObject();
        cJSON_AddStringToObject(d, "app", ownership_info.app_id);
        cJSON_AddNumberToObject(d, "priority", ownership_info.priority);
        telemetry_report_event(instance, TelemetryEventCanvasAcquire, d);
    }
}
