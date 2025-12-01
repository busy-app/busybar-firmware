/**
 * @file busy_timer_snapshot.h
 * @brief Data types and functions for BusyTimer snapshot handling.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BUSY_TIMER_CARD_ID_LEN (36)

#ifdef __cplusplus
extern "C" {
#endif
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
    char card_id[BUSY_TIMER_CARD_ID_LEN + 1];
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
} BusyTimerSnapshotInterval;

typedef struct {
    BusyTimerSnapshotType type;
    uint64_t timestamp_ms;
    union {
        BusyTimerSnapshotCommon common;
        BusyTimerSnapshotInfinite infinite;
        BusyTimerSnapshotSimple simple;
        BusyTimerSnapshotInterval interval;
    };
} BusyTimerSnapshot;

/**
 * @brief Convert a BusyTimerSnapshot object to its JSON representation.
 *
 * @warning The calling code is responsible for deleting the return value
 *          once it is not needed anymore.
 *
 * @param[in] snapshot pointer to the object to be serialised
 * @returns pointer to a dynamically allocated character string
 */
char* busy_timer_snapshot_serialize(const BusyTimerSnapshot* snapshot);

/**
 * @brief Load a BusyTimerSnapshot object from its JSON representation.
 *
 * @param[out] snapshot pointer to the object to load into (must be allocated)
 * @param[in] json_text pointer to a character string containing the JSON text
 * @returns @c true if the JSON could be successfully parsed, @c false otherwise
 */
bool busy_timer_snapshot_deserialize(BusyTimerSnapshot* snapshot, const char* json_text);

#ifdef __cplusplus
}
#endif
