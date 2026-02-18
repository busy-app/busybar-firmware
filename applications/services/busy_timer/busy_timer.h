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

typedef enum {
    BusyTimerEventTypeTick,
    BusyTimerEventTypeModeChanged,
    BusyTimerEventTypeStateChanged,
    BusyTimerEventTypeIntervalEnded,
    BusyTimerEventTypePaused,
    BusyTimerEventTypeProfileChanged,
    BusyTimerEventTypeMax,
} BusyTimerEventType;

typedef struct {
    BusyTimerState state;
    BusyTimerConfig config;
    uint32_t current_interval_idx;
} BusyTimerInfo;

typedef struct {
    BusyAppConfig app_config;
    BusyTimerConfig timer_config;
    bool is_demo_mode_enabled;
} BusyTimerGeneralConfig;

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
} BusyTimerEventProfileChanged;

typedef struct {
    BusyTimerEventType type;
    union {
        BusyTimerEventTick tick;
        BusyTimerEventModeChanged mode_changed;
        BusyTimerEventStateChanged state_changed;
        BusyTimerEventIntervalEnded interval_ended;
        BusyTimerEventPaused paused;
        BusyTimerEventProfileChanged profile_changed;
    };
} BusyTimerEvent;

FuriPubSub* busy_timer_get_pubsub(const BusyTimer* instance);

void busy_timer_start(BusyTimer* instance);

void busy_timer_stop(BusyTimer* instance);

void busy_timer_toggle(BusyTimer* instance);

void busy_timer_skip(BusyTimer* instance);

void busy_timer_add_time(BusyTimer* instance, int32_t time_mn);

void busy_timer_get_info(const BusyTimer* instance, BusyTimerInfo* info);

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot);

void busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot);

void busy_timer_get_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerProfile* profile);

void busy_timer_set_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerProfile* profile);

void busy_timer_load_profile(BusyTimer* instance, BusyTimerProfileId profile_id);

void busy_timer_get_general_config(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerGeneralConfig* config);

void busy_timer_set_general_config(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerGeneralConfig* config);

const char** busy_timer_get_mode_names(void);

#ifdef __cplusplus
}
#endif
