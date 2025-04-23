#pragma once

#include "busy_timer.h"

#include <toolbox/api_lock.h>

#include "time_macros.h"

#define TAG "BusyTimer"

#define WORK_TIME_DEFAULT_MN (20)
#define REST_TIME_DEFAULT_MN (5)
#define CYCLE_COUNT_DEFAULT  (3)

#define ENABLE_INTERVALS_DEFAULT (true)
#define ENABLE_AUTOSTART_DEFAULT (false)
#define ENABLE_SOUND_DEFAULT     (true)
#define ENABLE_SPEED_DEFAULT     (false)

#define WORK_TIME_MIN_MN (5)
#define WORK_TIME_MAX_MN H_TOM_M(8)
#define REST_TIME_MIN_MN (5)
#define REST_TIME_MAX_MN H_TOM_M(8)
#define CYCLE_COUNT_MIN  (2)
#define CYCLE_COUNT_MAX  (35)

#define SPEED_MULTIPLIER (60)

typedef enum {
    BusyTimerMessageTypeStart,
    BusyTimerMessageTypeStop,
    BusyTimerMessageTypeGetConfig,
    BusyTimerMessageTypeSetConfig,
    BusyTimerMessageTypeGetState,
    BusyTimerMessageTypeSetCallback,

    BusyTimerMessageTypeMax,
} BusyTimerMessageType;

typedef struct {
    BusyTimerCallback callback;
    void* context;
} BusyTimerCallbackInfo;

typedef union {
    BusyTimerState state;
    BusyTimerConfig* config;
    const BusyTimerConfig* config_c;
    const BusyTimerCallbackInfo* callback_info;
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
    uint32_t time_left_s;
    uint32_t cycles_left;
    BusyTimerConfig config;
    BusyTimerState state;
};
