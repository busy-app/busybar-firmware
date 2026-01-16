#pragma once

#include "busy_timer.h"
#include "busy_timer_settings.h"

#include <furi.h>

#include <sntp/sntp.h>
#include <toolbox/api_lock.h>

#define TAG "BusyTimer"

#define BUSY_TIMER_TIME_MIN_S M_TO_S(BUSY_TIMER_TIME_MIN_MN)
#define BUSY_TIMER_TIME_MAX_S M_TO_S(BUSY_TIMER_TIME_MAX_MN)

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
    FuriEventLoopTimer* debounce_timer;
    FuriMessageQueue* message_queue;
    FuriPubSub* event_pubsub;
    Sntp* sntp;
    uint64_t prev_tick_timestamp_ms;
    uint32_t current_interval_index;
    BusyTimerConfig config;
    BusyTimerTime time;
    BusyTimerMode mode;
    BusyTimerState state;
    BusyTimerProfileId profile_id;
    BusyTimerSnapshot user_snapshot;
    bool timer_running;
};
