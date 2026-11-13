#pragma once

#include "telemetry.h"
#include "telemetry_settings.h"

#include <furi.h>
#include <cjson/cJSON.h>
#include <stdatomic.h>

#include <toolbox/api_lock.h>

#include <mqtt/mqtt.h>
#include <canvas/canvas.h>
#include <wifi/wifi.h>
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
    TelemetryApiMessageTypeTimerRestored,
    TelemetryApiMessageTypeMax,
} TelemetryApiMessageType;

typedef struct {
    TelemetryEventType type;
    cJSON* data;
} TelemetryApiMessageReportEvent;

typedef struct {
    TelemetryStats* stats;
} TelemetryApiMessageGetStats;

typedef struct {
    bool is_enabled;
    char source[TELEMETRY_CONSENT_FIELD_LEN + 1];
    char text_version[TELEMETRY_CONSENT_FIELD_LEN + 1];
} TelemetryApiMessageSetEnabled;

typedef union {
    TelemetryApiMessageReportEvent report_event;
    TelemetryApiMessageGetStats get_stats;
    MqttStatus mqtt_status;
    TelemetryApiMessageSetEnabled set_enabled;
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

    cJSON* events[TELEMETRY_RING_CAPACITY];
    size_t events_head;
    size_t events_count;

    _Atomic bool is_connected;
    time_t offline_start_ms;
    bool has_offline_start;
    time_t last_push_ms;
    _Atomic bool is_enabled;
    bool is_disable_pending; // telemetry thread only

    // service thread state; events_dropped may be written by foreign threads
    uint32_t events_by_type[TelemetryEventMax];
    uint32_t batches_sent;
    uint32_t events_sent;
    _Atomic uint32_t events_dropped;

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
    BrightnessControlBrightnessMode last_brightness_mode;
    FuriPubSub* input_pubsub;
    Updater* updater;
    FuriState* updater_state;
    Matter* matter;
    FuriPubSub* mqtt_pubsub;
    FuriState* canvas_ownership_state;
    Wifi* wifi;
    FuriState* wifi_state;
    FuriPubSub* wifi_action_pubsub;

    // only touched in the canvas state callback
    bool last_canvas_active;
    char last_canvas_app[CANVAS_OWNER_APP_ID_MAX + 1];

    // only touched in the wifi state callback
    WifiState last_wifi_state;

    // busy timer session context, touched in the busy timer callback (and once at boot)
    bool timer_session_active;
    char timer_session_id[37];
    char timer_card_id[BUSY_TIMER_CARD_ID_LEN + 1];
    BusyTimerConfig timer_session_config;
    BusyTimerState timer_last_active_state;
    time_t timer_session_start_ms; // wall clock, for INFINITE (it does not tick)
    bool timer_restore_reported; // telemetry thread only

    // matter fabric count, touched in the matter callback
    FuriPubSub* matter_pubsub;
    size_t last_matter_fabrics;

    // touched from the input service thread
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

/**
 * @brief Fill a v4 UUID string (36 chars + NUL).
 */
void telemetry_generate_uuid(char out[37]);

/**
 * @brief Add timer_type and timer parameters (as the app names them) to @p d.
 */
void telemetry_add_timer_config(cJSON* d, const BusyTimerConfig* config);

/**
 * @brief Remember the version an update is installed from; reported after the reboot.
 */
void telemetry_fw_update_marker_save(const char* from_version);

/**
 * @brief Persist the running timer session (id and source) so a reboot can close it.
 */
void telemetry_timer_session_persist(const char* session_id, const char* source, uint32_t total_ms);

/**
 * @brief Load the persisted timer session. @returns @c true if one was saved.
 */
bool telemetry_timer_session_load(
    char session_id[37],
    char* source,
    size_t source_size,
    uint32_t* total_ms);

/**
 * @brief Forget the persisted timer session.
 */
void telemetry_timer_session_clear(void);
