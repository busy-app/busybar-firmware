/**
 * @file busy_timer_common.h
 * @brief Common BusyTimer types & macros.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

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

#define BUSY_TIMER_TIME_DEFAULT_MN      (20)
#define BUSY_TIMER_WORK_TIME_DEFAULT_MN (20)
#define BUSY_TIMER_REST_TIME_DEFAULT_MN (5)
#define BUSY_TIMER_CYCLE_COUNT_DEFAULT  (3)

#define BUSY_TIMER_ENABLE_AUTOSTART_DEFAULT (false)
#define BUSY_TIMER_ENABLE_DEMO_MODE_DEFAULT (false)

#define BUSY_TIMER_CARD_ID_LEN (36)
#define BUSY_TIMER_TITLE_LEN   (32 * 4)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BusyTimerModeInfinite,
    BusyTimerModeSimple,
    BusyTimerModeInterval,
    BusyTimerModeMax,
} BusyTimerMode;

typedef struct {
    int sort_order;
    char title[BUSY_TIMER_TITLE_LEN + 1];
    char card_id[BUSY_TIMER_CARD_ID_LEN + 1];
} BusyTimerMetadata;

typedef struct {
    uint32_t total_time_ms;
} BusyTimerSimpleConfig;

typedef struct {
    uint32_t work_time_ms;
    uint32_t rest_time_ms;
    uint32_t cycles_count;
    bool is_autostart_enabled;
} BusyTimerIntervalConfig;

typedef struct {
    uint32_t index;
    uint32_t time_total_ms;
    uint32_t time_left_ms;
} BusyTimerIntervalState;

typedef struct {
    BusyTimerMode mode;
    union {
        BusyTimerSimpleConfig simple;
        BusyTimerIntervalConfig interval;
    };
} BusyTimerConfig;

#ifdef __cplusplus
}
#endif
