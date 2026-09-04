#pragma once

#include <core/pubsub.h>

#include <busy/busy_common.h>

#include "busy_timer_common.h"
#include "busy_timer_profile.h"
#include "busy_timer_snapshot.h"

#define RECORD_BUSY_TIMER "busy_timer"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BusyTimer BusyTimer;

typedef enum {
    BusyTimerStateIdle,
    BusyTimerStateWork,
    BusyTimerStateRest,
    BusyTimerStateMax,
} BusyTimerState;

/**
 * @brief Source of a timer session start.
 */
typedef enum {
    BusyTimerSessionSourceUnknown =
        0, /**< Session restored from saved state at boot (or not tracked) */
    BusyTimerSessionSourceDevice, /**< Started from the device UI */
    BusyTimerSessionSourceHttpApi, /**< Started via the HTTP API (local web / companions) */
    BusyTimerSessionSourceIntegrationMatter, /**< Started via the Matter integration */
    BusyTimerSessionSourceIntegrationMqtt, /**< Started via an MQTT snapshot from the cloud/companion */
    BusyTimerSessionSourceMax, /**< Special value, internal use */
} BusyTimerSessionSource;

/**
 * @brief Outcome of a finished timer session.
 */
typedef enum {
    BusyTimerSessionOutcomeCompleted = 0, /**< Timer ran to its natural end */
    BusyTimerSessionOutcomeStopped, /**< Timer was stopped manually */
    BusyTimerSessionOutcomeMax, /**< Special value, internal use */
} BusyTimerSessionOutcome;

typedef enum {
    BusyTimerEventTypeTick,
    BusyTimerEventTypeModeChanged,
    BusyTimerEventTypeStateChanged,
    BusyTimerEventTypeIntervalEnded,
    BusyTimerEventTypePaused,
    BusyTimerEventTypeProfileChanged,
    BusyTimerEventTypeSnapshotCreated,
    BusyTimerEventTypeSessionStarted,
    BusyTimerEventTypeSessionEnded,
    BusyTimerEventTypeMax,
} BusyTimerEventType;

typedef struct {
    uint32_t time_elapsed_s;
    uint32_t time_remaining_s;
} BusyTimerEventTick;

typedef struct {
    BusyTimerMode mode;
} BusyTimerEventModeChanged;

typedef struct {
    BusyTimerState state;
} BusyTimerEventStateChanged;

typedef struct {
    bool is_forced;
} BusyTimerEventIntervalEnded;

typedef struct {
    bool is_paused;
} BusyTimerEventPaused;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerProfile profile;
} BusyTimerEventProfileChanged;

typedef struct {
    BusyTimerSnapshot snapshot;
} BusyTimerEventSnapshotCreated;

typedef struct {
    BusyTimerSessionSource source;
    BusyTimerProfileId profile_id;
    BusyAppConfig app_config;
    BusyTimerConfig timer_config;
    bool is_demo_mode_enabled;
} BusyTimerEventSessionStarted;

typedef struct {
    BusyTimerSessionOutcome outcome;
    BusyTimerSessionSource source;
    uint32_t time_elapsed_s;
    uint32_t current_interval_index;
} BusyTimerEventSessionEnded;

typedef struct {
    BusyTimerEventType type;
    union {
        BusyTimerEventTick tick;
        BusyTimerEventModeChanged mode_changed;
        BusyTimerEventStateChanged state_changed;
        BusyTimerEventIntervalEnded interval_ended;
        BusyTimerEventPaused paused;
        BusyTimerEventProfileChanged profile_changed;
        BusyTimerEventSnapshotCreated snapshot_created;
        BusyTimerEventSessionStarted session_started;
        BusyTimerEventSessionEnded session_ended;
    };
} BusyTimerEvent;

typedef struct {
    BusyTimerState state;
    BusyTimerConfig config;
    uint32_t current_interval_idx;
    uint32_t time_elapsed_s;
    BusyTimerSessionSource session_source;
} BusyTimerRunInfo;

typedef struct {
    BusyAppConfig app_config;
    BusyTimerConfig timer_config;
    bool is_demo_mode_enabled;
} BusyTimerPreset;

FuriPubSub* busy_timer_get_pubsub(const BusyTimer* instance);

void busy_timer_start(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerSessionSource source);

void busy_timer_stop(BusyTimer* instance);

void busy_timer_toggle(BusyTimer* instance);

void busy_timer_skip(BusyTimer* instance);

void busy_timer_finalize(BusyTimer* instance);

void busy_timer_add_time(BusyTimer* instance, int32_t time_minutes);

void busy_timer_get_run_info(const BusyTimer* instance, BusyTimerRunInfo* run_info);

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot);

void busy_timer_set_snapshot(
    BusyTimer* instance,
    const BusyTimerSnapshot* snapshot,
    BusyTimerSessionSource source);

const char* busy_timer_get_profile_name(BusyTimerProfileId profile_id);

void busy_timer_get_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerProfile* profile);

void busy_timer_set_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerProfile* profile);

void busy_timer_get_preset(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerPreset* preset);

void busy_timer_set_preset(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerPreset* preset);

const char** busy_timer_get_mode_names(void);

#ifdef __cplusplus
}
#endif
