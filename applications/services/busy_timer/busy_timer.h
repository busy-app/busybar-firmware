/**
 * @file busy_timer.h
 * @brief TBD
 */
#pragma once

#include "busy_timer_common.h"

#define RECORD_BUSY_TIMER "busy_timer"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BusyTimer BusyTimer;

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot);

void busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot);

#ifdef __cplusplus
}
#endif
