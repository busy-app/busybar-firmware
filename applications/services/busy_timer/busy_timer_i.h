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
    BusyTimerMessageTypeGetRunInfo,
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
    int32_t time_minutes;
} BusyTimerMessageAddTime;

typedef struct {
    BusyTimerSnapshot* snapshot;
} BusyTimerMessageGetSnapshot;

typedef struct {
    const BusyTimerSnapshot* snapshot;
} BusyTimerMessageSetSnapshot;

typedef struct {
    BusyTimerRunInfo* run_info;
} BusyTimerMessageGetRunInfo;

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
} BusyTimerMessageLoadProfile;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerPreset* preset;
} BusyTimerMessageGetPreset;

typedef struct {
    BusyTimerProfileId profile_id;
    const BusyTimerPreset* preset;
} BusyTimerMessageSetPreset;

typedef union {
    BusyTimerMessageAddTime add_time;
    BusyTimerMessageGetSnapshot get_snapshot;
    BusyTimerMessageSetSnapshot set_snapshot;
    BusyTimerMessageGetRunInfo get_run_info;
    BusyTimerMessageGetProfile get_profile;
    BusyTimerMessageSetProfile set_profile;
    BusyTimerMessageLoadProfile load_profile;
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
    BusyTimerSnapshot user_snapshot;
    BusyTimerSettings settings[BusyTimerProfileIdMax];
    // TODO FW-635: Refactor & simplify internals ---->
    BusyTimerState state;
    time_t prev_tick_timestamp_ms;
    uint32_t current_interval_index;
    uint32_t time_elapsed_s;
    uint32_t time_remaining_s;
    BusyAppConfig app_config;
    BusyTimerConfig timer_config;
    char card_id[BUSY_TIMER_CARD_ID_LEN + 1];
    // <----- Refactor section ends
    bool is_timer_running;
    bool is_demo_mode_enabled;
};
