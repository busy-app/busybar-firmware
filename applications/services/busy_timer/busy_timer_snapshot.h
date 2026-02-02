/**
 * @file busy_timer_snapshot.h
 * @brief Data types and functions for BusyTimer snapshot handling.
 */
#pragma once

#include "busy_timer_common.h"

#ifdef __cplusplus
extern "C" {
#endif
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
    time_t timestamp_ms;
    union {
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

/**
 * @brief Check whether a BusyTimerSnapshot object represents a valid state.
 *
 * @param[in] snapshot pointer to the object to be validated
 * @returns @c true if the object is valid, @c false otherwise
 */
bool busy_timer_snapshot_is_valid(const BusyTimerSnapshot* snapshot);

#ifdef __cplusplus
}
#endif
