#pragma once

#include <furi.h>

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

typedef struct {
    uint32_t total_time_mn;
    uint32_t work_time_mn;
    uint32_t rest_time_mn;
    bool enable_intervals;
    bool enable_autostart_work;
    bool enable_autostart_rest;
    bool enable_autorestart;
    bool enable_sound;
    bool enable_speed;
} BusyTimerConfig;

typedef void (*BusyTimerCallback)(void* context);

BusyTimer* busy_timer_alloc(void);

void busy_timer_free(BusyTimer* instance);

BusyTimerState busy_timer_get_state(const BusyTimer* instance);

void busy_timer_get_config(const BusyTimer* instance, BusyTimerConfig* config);

void busy_timer_set_config(const BusyTimer* instance, const BusyTimerConfig* config);

// void busy_timer_start(BusyTimer* instance);
// void busy_timer_stop(BusyTimer* instance);
// void busy_timer_next_state(BusyTimer* instance, bool skip_event);
// void busy_timer_pause(BusyTimer* instance);
// void busy_timer_resume(BusyTimer* instance);
// void busy_timer_toggle(BusyTimer* instance);
// bool busy_timer_is_running(BusyTimer* instance);

#ifdef __cplusplus
}
#endif
