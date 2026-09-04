/**
 * @file telemetry.h
 * @brief Telemetry collection & reporting service API.
 *
 * The telemetry service collects lightweight device events and reports them to the
 * backend over a device-scope MQTT topic. It does not require an account to be linked
 * to the device. Events are batched and published periodically; high-priority events
 * trigger an immediate (rate-limited) flush.
 */
#pragma once

#include <cjson/cJSON.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for the telemetry service instance access.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_TELEMETRY);`
 */
#define RECORD_TELEMETRY "telemetry"

/**
 * @brief Opaque data type for the telemetry service instance.
 */
typedef struct Telemetry Telemetry;

/**
 * @brief Telemetry event delivery priority.
 */
typedef enum {
    TelemetryPriorityLow = 0, /**< p0: droppable when offline, aggregated */
    TelemetryPriorityBatch = 1, /**< p1: buffered and flushed periodically */
    TelemetryPriorityPush = 2, /**< p2: buffered, pushed immediately (rate-limited) */
} TelemetryPriority;

/**
 * @brief Telemetry event types.
 *
 * Each event is reported as `{"t":"<name>","ts":<ms>,"p":<priority>,"d":{...}}`
 * inside a batched message on the device-scope telemetry topic.
 */
typedef enum {
    TelemetryEventDeviceBoot, /**< Device boot / firmware info (p2) */
    TelemetryEventFwUpdate, /**< Firmware update completed (p2) */
    TelemetryEventTimerSessionStart, /**< Busy timer session started (p1) */
    TelemetryEventTimerSessionEnd, /**< Busy timer session ended / interrupted (p2) */
    TelemetryEventTimerTheme, /**< BUSY/CUSTOM timer theme changed (p0) */
    TelemetryEventAppStart, /**< Application started (p1) */
    TelemetryEventAppStop, /**< Application stopped (p1) */
    TelemetryEventSettingBrightness, /**< Display brightness changed (p0) */
    TelemetryEventSettingVolume, /**< Audio volume changed (p0) */
    TelemetryEventInputSwitch, /**< Mode switch position changed (p0) */
    TelemetryEventPowerTransition, /**< Charging / USB power state changed (p1) */
    TelemetryEventNetOnline, /**< MQTT (re)connected (p2) */
    TelemetryEventNetOffline, /**< MQTT disconnected (p2) */
    TelemetryEventNetOfflineDuration, /**< Reported offline duration on reconnect (p2) */
    TelemetryEventAccountLink, /**< Account linked (p2) */
    TelemetryEventAccountUnlink, /**< Account unlinked (p2) */
    TelemetryEventCanvasAcquire, /**< Canvas acquired by an HTTP API app (p1) */
    TelemetryEventCanvasRelease, /**< Canvas released (no HTTP API content) (p1) */
    TelemetryEventMax, /**< Special value, internal use */
} TelemetryEventType;

/**
 * @brief Report a telemetry event.
 *
 * @note Takes ownership of @p data (a JSON object) — it is freed by the service.
 *       Non-blocking; the event is dropped (and @p data freed) if the internal
 *       queue is full or telemetry collection is disabled.
 *
 * @param[in] instance telemetry service instance
 * @param[in] type event type
 * @param[in] data event payload (the "d" object), may be NULL
 */
void telemetry_report_event(Telemetry* instance, TelemetryEventType type, cJSON* data);

/**
 * @brief Telemetry service statistics snapshot (debug/status reporting).
 */
typedef struct {
    bool is_enabled; /**< Whether collection is enabled */
    bool is_connected; /**< Whether the device MQTT connection is up */
    uint32_t buffered_events; /**< Events currently buffered, awaiting flush */
    uint32_t batches_sent; /**< Batch messages published to the backend */
    uint32_t events_sent; /**< Events published to the backend */
    uint32_t events_dropped; /**< Events dropped (disabled, offline p0, queue/ring full) */
    uint32_t events_by_type[TelemetryEventMax]; /**< Events enqueued per type */
} TelemetryStats;

/**
 * @brief Query telemetry service statistics.
 *
 * Blocking call; safe to use from any thread.
 *
 * @param[in] instance telemetry service instance
 * @param[out] stats pre-allocated structure to fill
 */
void telemetry_get_stats(Telemetry* instance, TelemetryStats* stats);

/**
 * @brief Get the wire name of a telemetry event type (as used in the "t" field).
 * @param[in] type event type
 * @returns event type name string
 */
const char* telemetry_event_type_name(TelemetryEventType type);

/**
 * @brief Check whether telemetry collection is enabled (user opt-out).
 * @param[in] instance telemetry service instance
 * @returns @c true if enabled, @c false otherwise
 */
bool telemetry_is_enabled(Telemetry* instance);

/**
 * @brief Enable or disable telemetry collection (user opt-out).
 *
 * The setting is persisted; buffered events are dropped when disabling.
 *
 * @param[in] instance telemetry service instance
 * @param[in] enabled @c true to enable, @c false to disable
 */
void telemetry_set_enabled(Telemetry* instance, bool enabled);

#ifdef __cplusplus
}
#endif
