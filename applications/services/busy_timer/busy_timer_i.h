#pragma once

#include "busy_timer.h"

#include <toolbox/api_lock.h>

#define TAG "BusyTimer"

#define BUSY_TIMER_TIME_MIN_S M_TO_S(BUSY_TIMER_TIME_MIN_MN)
#define BUSY_TIMER_TIME_MAX_S M_TO_S(BUSY_TIMER_TIME_MAX_MN)

#define TIME_DEFAULT_MN      (20)
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
    BusyTimerMessageTypeAddTime,
    BusyTimerMessageTypeToggle,
    BusyTimerMessageTypeSkip,
    BusyTimerMessageTypeGetSnapshot,
    BusyTimerMessageTypeSetSnapshot,

    BusyTimerMessageTypeMax,
} BusyTimerMessageType;

typedef union {
    BusyTimerState* state;
    BusyTimerTime* time;
    BusyTimerCycles* cycles;
    BusyTimerConfig* config;
    const BusyTimerConfig* config_c;
    int32_t add_time_mn;
    BusyTimerSnapshot* snapshot;
    const BusyTimerSnapshot* snapshot_c;
} BusyTimerMessageData;

typedef struct {
    BusyTimerMessageType type;
    BusyTimerMessageData data;
    FuriApiLock lock;
} BusyTimerMessage;

struct BusyTimer {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* poll_timer;
    FuriMessageQueue* message_queue;
    FuriPubSub* event_pubsub;
    void* callback_context;
    uint64_t prev_tick_timestamp_ms;
    uint64_t prev_snaphot_timestamp_ms;
    uint32_t cycles_done;
    BusyTimerConfig config;
    BusyTimerTime time;
    BusyTimerMode mode;
    BusyTimerState state;
    bool timer_running;
};
