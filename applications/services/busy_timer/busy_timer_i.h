#pragma once

#include "busy_timer.h"
#include "settings/busy_timer_settings.h"

#include <furi.h>

#include <mqtt/mqtt.h>

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
    BusyTimerMessageTypeGetAppConfig,
    BusyTimerMessageTypeSetAppConfig,
    BusyTimerMessageTypeSaveConfig,
    BusyTimerMessageTypeGetState,
    BusyTimerMessageTypeGetTime,
    BusyTimerMessageTypeGetCycles,
    BusyTimerMessageTypeAddTime,
    BusyTimerMessageTypeToggle,
    BusyTimerMessageTypeSkip,
    BusyTimerMessageTypeGetSnapshot,
    BusyTimerMessageTypeSetSnapshot,
    BusyTimerMessageTypeGetProfile,
    BusyTimerMessageTypeSetProfile,
    BusyTimerMessageTypeLoadProfile,

    BusyTimerMessageTypeMax,
} BusyTimerMessageType;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerProfile* profile;
} BusyTimerMessageGetProfile;

typedef struct {
    BusyTimerProfileId profile_id;
    const BusyTimerProfile* profile;
} BusyTimerMessageSetProfile;

typedef union {
    BusyTimerState* state;
    BusyTimerTime* time;
    BusyTimerCycles* cycles;
    BusyTimerConfig* config;
    const BusyTimerConfig* config_c;
    BusyAppConfig* app_config;
    const BusyAppConfig* app_config_c;
    int32_t add_time_mn;
    BusyTimerSnapshot* snapshot;
    const BusyTimerSnapshot* snapshot_c;
    BusyTimerMessageGetProfile get_profile;
    BusyTimerMessageSetProfile set_profile;
    BusyTimerProfileId profile_id;
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
    Mqtt* mqtt;
    time_t prev_tick_timestamp_ms;
    uint32_t current_interval_index;
    BusyTimerSettings settings;
    BusyTimerTime time;
    BusyTimerMode mode;
    BusyTimerState state;
    BusyTimerProfileId profile_id;
    BusyTimerSnapshot user_snapshot;
    bool timer_running;
};
