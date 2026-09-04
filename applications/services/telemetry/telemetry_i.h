#pragma once

#include "telemetry.h"
#include "telemetry_settings.h"

#include <furi.h>
#include <cjson/cJSON.h>
#include <stdatomic.h>

#include <toolbox/api_lock.h>

#include <mqtt/mqtt.h>
#include <canvas/canvas.h>
#include <loader/loader.h>
#include <busy_timer/busy_timer.h>
#include <power/power_service/power.h>
#include <audio/audio.h>
#include <brightness_control/brightness_control.h>
#include <input/input.h>
#include <updater/updater.h>
#include <matter/matter.h>

#define TAG "Telemetry"

#define TELEMETRY_API_QUEUE_SIZE       (16)
#define TELEMETRY_RING_CAPACITY        (32)
#define TELEMETRY_FLUSH_INTERVAL_MS    (15 * 60 * 1000)
#define TELEMETRY_FLUSH_BATCH_EVENTS   (32)
#define TELEMETRY_PUSH_MIN_INTERVAL_MS (5000)
#define TELEMETRY_MQTT_TOPIC           "telemetry"
#define TELEMETRY_MQTT_QOS             MqttQosAtLeastOnce
#define TELEMETRY_SCHEMA               1

typedef enum {
    TelemetryApiMessageTypeReportEvent,
    TelemetryApiMessageTypeMqttStatus,
    TelemetryApiMessageTypePowerEvent,
    TelemetryApiMessageTypeAudioEvent,
    TelemetryApiMessageTypeSetEnabled,
    TelemetryApiMessageTypeGetStats,
    TelemetryApiMessageTypeFlush,
    TelemetryApiMessageTypeMax,
} TelemetryApiMessageType;

typedef struct {
    TelemetryEventType type;
    cJSON* data;
} TelemetryApiMessageReportEvent;

typedef struct {
    TelemetryStats* stats;
} TelemetryApiMessageGetStats;

typedef union {
    TelemetryApiMessageReportEvent report_event;
    TelemetryApiMessageGetStats get_stats;
    MqttStatus mqtt_status;
    bool is_enabled;
} TelemetryApiMessageData;

typedef struct {
    TelemetryApiMessageType type;
    TelemetryApiMessageData data;
    FuriApiLock lock;
} TelemetryApiMessage;

struct Telemetry {
    FuriEventLoop* event_loop;
    FuriMessageQueue* api_queue;
    FuriEventLoopTimer* flush_timer;
    Mqtt* mqtt;

    // ring buffer of pending event objects
    cJSON* events[TELEMETRY_RING_CAPACITY];
    size_t events_head;
    size_t events_count;

    // connectivity & rate limiting
    bool is_connected;
    time_t offline_start_ms;
    bool has_offline_start;
    time_t last_push_ms;
    bool is_enabled;

    // statistics (events_by_type & aggregates touched on the service thread;
    // events_dropped is atomic — incremented from foreign threads on queue-full)
    uint32_t events_by_type[TelemetryEventMax];
    uint32_t batches_sent;
    uint32_t events_sent;
    _Atomic uint32_t events_dropped;

    // collector handles & state
    Loader* loader;
    FuriPubSub* loader_pubsub;
    BusyTimer* busy_timer;
    FuriPubSub* busy_timer_pubsub;
    Power* power;
    FuriPubSub* power_pubsub;
    Audio* audio;
    FuriPubSub* audio_pubsub;
    BrightnessControl* brightness_control;
    FuriState* brightness_state;
    uint8_t last_brightness_value;
    FuriPubSub* input_pubsub;
    Updater* updater;
    FuriState* updater_state;
    Matter* matter;
    FuriPubSub* mqtt_pubsub;
    FuriState* canvas_ownership_state;

    // canvas ownership dedup (only touched in the canvas state callback)
    bool last_canvas_active;
    char last_canvas_app[CANVAS_OWNER_APP_ID_MAX + 1];

    // input aggregate counters (touched from the input service thread)
    _Atomic uint32_t input_ok;
    _Atomic uint32_t input_back;
    _Atomic uint32_t input_start;
    _Atomic uint32_t input_wheel_up;
    _Atomic uint32_t input_wheel_down;
};

/**
 * @brief Open records and subscribe to event sources.
 * Called once from telemetry_alloc() on the telemetry service thread.
 */
void telemetry_collectors_init(Telemetry* instance);
