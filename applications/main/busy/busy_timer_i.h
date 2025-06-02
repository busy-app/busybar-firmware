#pragma once

#include "busy_timer.h"

#include <toolbox/api_lock.h>

#define TAG "BusyTimer"

#define BUSY_TIMER_TIME_MIN_S M_TO_S(BUSY_TIMER_TIME_MIN_MN)
#define BUSY_TIMER_TIME_MAX_S M_TO_S(BUSY_TIMER_TIME_MAX_MN)

#define WORK_TIME_DEFAULT_MN (20)
#define REST_TIME_DEFAULT_MN (5)
#define CYCLE_COUNT_DEFAULT  (3)

#define ENABLE_INTERVALS_DEFAULT (true)
#define ENABLE_AUTOSTART_DEFAULT (false)
#define ENABLE_SPEED_DEFAULT     (false)

#define SPEED_MULTIPLIER (60)

typedef enum {
    BusyTimerMessageTypeStart,
    BusyTimerMessageTypeStop,
    BusyTimerMessageTypeGetConfig,
    BusyTimerMessageTypeSetConfig,
    BusyTimerMessageTypeGetState,
    BusyTimerMessageTypeGetTime,
    BusyTimerMessageTypeGetCycles,
    BusyTimerMessageTypeSetCallback,
    BusyTimerMessageTypeAddTime,
    BusyTimerMessageTypeToggle,
    BusyTimerMessageTypeSkip,

    BusyTimerMessageTypeMax,
} BusyTimerMessageType;

typedef struct {
    BusyTimerCallback callback;
    void* context;
} BusyTimerCallbackInfo;

typedef union {
    BusyTimerState* state;
    BusyTimerTime* time;
    BusyTimerCycles* cycles;
    BusyTimerConfig* config;
    const BusyTimerConfig* config_c;
    const BusyTimerCallbackInfo* callback_info;
    int32_t add_time_mn;
} BusyTimerMessageData;

typedef struct {
    BusyTimerMessageType type;
    BusyTimerMessageData data;
    FuriApiLock lock;
} BusyTimerMessage;

struct BusyTimer {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriMessageQueue* message_queue;
    BusyTimerCallback callback;
    void* callback_context;
    uint32_t cycles_done;
    BusyTimerConfig config;
    BusyTimerTime time;
    BusyTimerMode mode;
    BusyTimerState state;
    bool next_state_forced;
};
