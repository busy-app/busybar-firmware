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
    TelemetryEventAppStart, /**< Application started (p1) */
    TelemetryEventAppStop, /**< Application stopped (p1) */
    TelemetryEventSettingBrightness, /**< Display brightness changed (p0) */
    TelemetryEventSettingTheme, /**< BUSY/CUSTOM theme changed (p0) */
    TelemetryEventSettingVolume, /**< Audio volume changed (p0) */
    TelemetryEventInputSwitch, /**< Mode switch position changed (p0) */
    TelemetryEventPowerTransition, /**< Charging / USB power state changed (p1) */
    TelemetryEventNetOnline, /**< MQTT (re)connected (p2) */
    TelemetryEventNetOffline, /**< MQTT disconnected (p2) */
    TelemetryEventNetOfflineDuration, /**< Reported offline duration on reconnect (p2) */
    TelemetryEventAccountLink, /**< Account linked (p2) */
    TelemetryEventAccountUnlink, /**< Account unlinked (p2) */
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
