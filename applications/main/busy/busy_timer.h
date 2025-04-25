#pragma once

#include <furi.h>

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
    BusyTimerEventTypeStateChanged,
    BusyTimerEventTypeMax,
} BusyTimerEventType;

typedef struct {
    BusyTimerEventType type;
    union {
        uint32_t time_s;
        BusyTimerState state;
    };
} BusyTimerEvent;

typedef struct {
    uint32_t work_time_mn;
    uint32_t rest_time_mn;
    uint32_t cycle_count;
    bool enable_intervals;
    bool enable_autostart;
    bool enable_sound;
    bool enable_speed;
} BusyTimerConfig;

typedef void (*BusyTimerCallback)(const BusyTimerEvent* event, void* context);

BusyTimer* busy_timer_alloc(void);

void busy_timer_free(BusyTimer* instance);

void busy_timer_set_callback(BusyTimer* instance, BusyTimerCallback callback, void* context);

BusyTimerState busy_timer_get_state(const BusyTimer* instance);

void busy_timer_get_config(const BusyTimer* instance, BusyTimerConfig* config);

void busy_timer_set_config(const BusyTimer* instance, const BusyTimerConfig* config);

void busy_timer_start(BusyTimer* instance);

void busy_timer_stop(BusyTimer* instance);

// void busy_timer_next_state(BusyTimer* instance, bool skip_event);
// void busy_timer_pause(BusyTimer* instance);
// void busy_timer_resume(BusyTimer* instance);
// void busy_timer_toggle(BusyTimer* instance);
// bool busy_timer_is_running(BusyTimer* instance);

void busy_timer_add_time(BusyTimer* instance, int32_t time_mn);

#ifdef __cplusplus
}
#endif
