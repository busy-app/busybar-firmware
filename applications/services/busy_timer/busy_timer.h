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

typedef enum {
    BusyTimerProfileIdBusy,
    BusyTimerProfileIdCustom,
    BusyTimerProfileIdMax,
} BusyTimerProfileId;

typedef struct {
    uint32_t elapsed_s;
    uint32_t remain_s;
} BusyTimerTime;

typedef struct {
    uint32_t current_idx;
    uint32_t total_count;
} BusyTimerCycles;

typedef struct {
    BusyTimerProfileSettings timer_settings;
} BusyTimerInfo;

typedef struct {
    bool is_paused;
} BusyTimerEventPaused;

typedef struct {
    BusyTimerProfileId profile_id;
} BusyTimerEventProfileChanged;

typedef struct {
    BusyTimerEventType type;
    union {
        BusyTimerTime time;
        BusyTimerMode mode;
        BusyTimerState state;
        bool is_force_ended;
        BusyTimerEventPaused paused;
        BusyTimerEventProfileChanged profile_changed;
    };
} BusyTimerEvent;

FuriPubSub* busy_timer_get_pubsub(const BusyTimer* instance);

BusyTimerState busy_timer_get_state(const BusyTimer* instance);

void busy_timer_get_time(const BusyTimer* instance, BusyTimerTime* time);

void busy_timer_get_cycles(const BusyTimer* instance, BusyTimerCycles* cycles);

void busy_timer_get_info(const BusyTimer* instance, BusyTimerInfo* info);

void busy_timer_start(BusyTimer* instance);

void busy_timer_stop(BusyTimer* instance);

void busy_timer_toggle(BusyTimer* instance);

void busy_timer_skip(BusyTimer* instance);

void busy_timer_add_time(BusyTimer* instance, int32_t time_mn);

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

const char** busy_timer_get_mode_names(void);

#ifdef __cplusplus
}
#endif
