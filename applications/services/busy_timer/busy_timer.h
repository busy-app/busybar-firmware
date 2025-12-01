#pragma once

#include <furi.h>

#include "busy_timer_snapshot.h"

#include "time_macros.h"

#define BUSY_TIMER_TIME_MIN_MN       (5)
#define BUSY_TIMER_TIME_MAX_MN       H_TO_M(24)
#define BUSY_TIMER_TIME_INCREMENT_MN (5)

#define BUSY_TIMER_WORK_TIME_MIN_MN (5)
#define BUSY_TIMER_WORK_TIME_MAX_MN H_TO_M(8)
#define BUSY_TIMER_REST_TIME_MIN_MN (5)
#define BUSY_TIMER_REST_TIME_MAX_MN H_TO_M(8)

#define BUSY_TIMER_CYCLE_COUNT_MIN (2)
#define BUSY_TIMER_CYCLE_COUNT_MAX (35)
#define BUSY_TIMER_CYCLE_INCREMENT (1)

#define RECORD_BUSY_TIMER "busy_timer"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BusyTimer BusyTimer;

typedef enum {
    BusyTimerModeInfinite,
    BusyTimerModeSimple,
    BusyTimerModeInterval,
    BusyTimerModeMax,
} BusyTimerMode;

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
    BusyTimerEventTypeTimerPaused,
    BusyTimerEventTypeUserInteracted,
    BusyTimerEventTypeMax,
} BusyTimerEventType;

typedef struct {
    uint32_t elapsed_s;
    uint32_t remain_s;
} BusyTimerTime;

typedef struct {
    uint32_t done_count;
    uint32_t total_count;
} BusyTimerCycles;

typedef struct {
    bool is_paused;
} BusyTimerEventTimerPaused;

typedef struct {
    BusyTimerSnapshot snapshot;
} BusyTimerEventUserInteracted;

typedef struct {
    BusyTimerEventType type;
    union {
        BusyTimerTime time;
        BusyTimerMode mode;
        BusyTimerState state;
        bool is_force_ended;
        BusyTimerEventTimerPaused timer_paused;
        BusyTimerEventUserInteracted user_interacted;
    };
} BusyTimerEvent;

typedef struct {
    BusyTimerMode mode;
    uint32_t time_mn;
    uint32_t work_time_mn;
    uint32_t rest_time_mn;
    uint32_t cycle_count;
    bool enable_intervals;
    bool enable_autostart;
    bool enable_demo_mode;
} BusyTimerConfig;

FuriPubSub* busy_timer_get_pubsub(const BusyTimer* instance);

BusyTimerState busy_timer_get_state(const BusyTimer* instance);

void busy_timer_get_time(const BusyTimer* instance, BusyTimerTime* time);

void busy_timer_get_cycles(const BusyTimer* instance, BusyTimerCycles* cycles);

void busy_timer_get_config(const BusyTimer* instance, BusyTimerConfig* config);

void busy_timer_set_config(const BusyTimer* instance, const BusyTimerConfig* config);

void busy_timer_start(BusyTimer* instance);

void busy_timer_stop(BusyTimer* instance);

void busy_timer_toggle(BusyTimer* instance);

void busy_timer_skip(BusyTimer* instance);

void busy_timer_add_time(BusyTimer* instance, int32_t time_mn);

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot);

void busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot);

const char** busy_timer_get_mode_names(void);

#ifdef __cplusplus
}
#endif
