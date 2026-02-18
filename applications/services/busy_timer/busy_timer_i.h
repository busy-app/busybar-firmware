#pragma once

#include "busy_timer.h"
#include "settings/busy_timer_settings.h"

#include <furi.h>

#include <mqtt/mqtt.h>

#include <toolbox/api_lock.h>

#define TAG "BusyTimer"

#define BUSY_TIMER_TIME_MIN_S M_TO_S(BUSY_TIMER_TIME_MIN_MN)
#define BUSY_TIMER_TIME_MAX_S M_TO_S(BUSY_TIMER_TIME_MAX_MN)

typedef enum {
    BusyTimerMessageTypeStart,
    BusyTimerMessageTypeStop,
    BusyTimerMessageTypeAddTime,
    BusyTimerMessageTypeToggle,
    BusyTimerMessageTypeSkip,
    BusyTimerMessageTypeGetInfo,
    BusyTimerMessageTypeGetSnapshot,
    BusyTimerMessageTypeSetSnapshot,
    BusyTimerMessageTypeGetProfile,
    BusyTimerMessageTypeSetProfile,
    BusyTimerMessageTypeLoadProfile,
    BusyTimerMessageTypeGetPreset,
    BusyTimerMessageTypeSetPreset,

    BusyTimerMessageTypeMax,
} BusyTimerMessageType;

typedef struct {
    BusyTimerInfo* info;
} BusyTimerMessageGetInfo;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerProfile* profile;
} BusyTimerMessageGetProfile;

typedef struct {
    BusyTimerProfileId profile_id;
    const BusyTimerProfile* profile;
} BusyTimerMessageSetProfile;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerPreset* preset;
} BusyTimerMessageGetPreset;

typedef struct {
    BusyTimerProfileId profile_id;
    const BusyTimerPreset* preset;
} BusyTimerMessageSetPreset;

typedef union {
    int32_t add_time_mn;
    BusyTimerSnapshot* snapshot;
    const BusyTimerSnapshot* snapshot_c;
    BusyTimerMessageGetInfo get_info;
    BusyTimerMessageGetProfile get_profile;
    BusyTimerMessageSetProfile set_profile;
    BusyTimerProfileId profile_id;
    BusyTimerMessageGetPreset get_preset;
    BusyTimerMessageSetPreset set_preset;
} BusyTimerMessageData;

typedef struct {
    BusyTimerMessageType type;
    BusyTimerMessageData data;
    FuriApiLock lock;
} BusyTimerMessage;

typedef enum {
    BusyTimerSetProfileResultRejectedInvalid,
    BusyTimerSetProfileResultRejectedOutdated,
    BusyTimerSetProfileResultRejectedFuture,
    BusyTimerSetProfileResultRejectedOwn,
    BusyTimerSetProfileResultAccepted,
    BusyTimerSetProfileResultMax,
} BusyTimerSetProfileResult;

struct BusyTimer {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* poll_timer;
    FuriEventLoopTimer* debounce_timer;
    FuriMessageQueue* message_queue;
    FuriPubSub* event_pubsub;
    Mqtt* mqtt;
    // TODO: Refactor the mess below
    BusyTimerMode mode;
    BusyTimerState state;
    time_t prev_tick_timestamp_ms;
    uint32_t current_interval_index;
    uint32_t time_elapsed_s;
    uint32_t time_remaining_s;
    union {
        BusyTimerSimpleConfig simple_config;
        BusyTimerIntervalConfig interval_config;
    };
    BusyTimerSettings settings[BusyTimerProfileIdMax];
    BusyTimerSnapshot user_snapshot;
    BusyAppConfig app_config;
    char card_id[BUSY_TIMER_CARD_ID_LEN + 1];
    bool timer_running;
    bool is_demo_mode_enabled;
};
