/**
 * @file busy_timer.h
 * @brief TBD
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_BUSY_TIMER "busy_timer"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BusyTimer BusyTimer;

typedef struct {
    uint32_t work_time_ms;
    uint32_t rest_time_ms;
    uint32_t cycles_count;
    bool is_autostart_enabled;
} BusyTimerIntervalSettings;

typedef struct {
    uint32_t index;
    uint32_t time_total_ms;
    uint32_t time_left_ms;
} BusyTimerIntervalState;

typedef enum {
    BusyTimerSnapshotTypeNotStarted,
    BusyTimerSnapshotTypeInfinite,
    BusyTimerSnapshotTypeSimple,
    BusyTimerSnapshotTypeInterval,
    BusyTimerSnapshotTypeMax,
} BusyTimerSnapshotType;

typedef struct {
    uint32_t card_id;
    bool is_paused;
} BusyTimerSnapshotCommon;

typedef struct {
    BusyTimerSnapshotCommon common;
} BusyTimerSnapshotInfinite;

typedef struct {
    BusyTimerSnapshotCommon common;
    uint32_t time_left_ms;
} BusyTimerSnapshotSimple;

typedef struct {
    BusyTimerSnapshotCommon common;
    BusyTimerIntervalState state;
    BusyTimerIntervalSettings settings;
    bool has_settings;
} BusyTimerSnapshotInterval;

typedef struct {
    BusyTimerSnapshotType type;
    uint64_t timestamp_ms;
    union {
        BusyTimerSnapshotInfinite infinite;
        BusyTimerSnapshotSimple simple;
        BusyTimerSnapshotInterval interval;
    };
} BusyTimerSnapshot;

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot);

bool busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot);

#ifdef __cplusplus
}
#endif
