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
    BusyTimerApiMessageTypeStart,
    BusyTimerApiMessageTypeStop,
    BusyTimerApiMessageTypeAddTime,
    BusyTimerApiMessageTypeToggle,
    BusyTimerApiMessageTypeSkip,
    BusyTimerApiMessageTypeFinalize,
    BusyTimerApiMessageTypeGetRunInfo,
    BusyTimerApiMessageTypeGetSnapshot,
    BusyTimerApiMessageTypeSetSnapshot,
    BusyTimerApiMessageTypeGetProfile,
    BusyTimerApiMessageTypeSetProfile,
    BusyTimerApiMessageTypeLoadProfile,
    BusyTimerApiMessageTypeGetPreset,
    BusyTimerApiMessageTypeSetPreset,

    BusyTimerApiMessageTypeMax,
} BusyTimerApiMessageType;

typedef struct {
    int32_t time_minutes;
} BusyTimerApiMessageAddTime;

typedef struct {
    BusyTimerSnapshot* snapshot;
} BusyTimerApiMessageGetSnapshot;

typedef struct {
    BusyTimerSnapshot snapshot;
} BusyTimerApiMessageSetSnapshot;

typedef struct {
    BusyTimerRunInfo* run_info;
} BusyTimerApiMessageGetRunInfo;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerProfile* profile;
} BusyTimerApiMessageGetProfile;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerProfile profile;
} BusyTimerApiMessageSetProfile;

typedef struct {
    BusyTimerProfileId profile_id;
} BusyTimerApiMessageLoadProfile;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerPreset* preset;
} BusyTimerApiMessageGetPreset;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerPreset preset;
} BusyTimerApiMessageSetPreset;

typedef union {
    BusyTimerApiMessageAddTime add_time;
    BusyTimerApiMessageGetSnapshot get_snapshot;
    BusyTimerApiMessageSetSnapshot set_snapshot;
    BusyTimerApiMessageGetRunInfo get_run_info;
    BusyTimerApiMessageGetProfile get_profile;
    BusyTimerApiMessageSetProfile set_profile;
    BusyTimerApiMessageLoadProfile load_profile;
    BusyTimerApiMessageGetPreset get_preset;
    BusyTimerApiMessageSetPreset set_preset;
} BusyTimerApiMessageData;

typedef struct {
    BusyTimerApiMessageType type;
    BusyTimerApiMessageData data;
    FuriApiLock lock;
} BusyTimerApiMessage;

typedef enum {
    BusyTimerSetProfileResultRejectedInvalid,
    BusyTimerSetProfileResultRejectedOutdated,
    BusyTimerSetProfileResultRejectedOwn,
    BusyTimerSetProfileResultAccepted,
    BusyTimerSetProfileResultMax,
} BusyTimerSetProfileResult;

struct BusyTimer {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* poll_timer;
    FuriEventLoopTimer* snapshot_timer;
    FuriEventLoopTimer* profile_timer;
    FuriMessageQueue* api_queue;
    FuriPubSub* event_pubsub;
    Mqtt* mqtt;
    BusyTimerSnapshot user_snapshot;
    BusyTimerSettings settings[BusyTimerProfileIdMax];
    // TODO FW-635: Refactor & simplify internals ---->
    BusyTimerState state;
    BusyAppConfig app_config;
    BusyTimerConfig timer_config;
    time_t prev_tick_timestamp_ms;
    uint32_t current_interval_index;
    uint32_t time_elapsed_s;
    uint32_t time_remaining_s;
    char card_id[BUSY_TIMER_CARD_ID_LEN + 1];
    // <----- Refactor section ends
    bool is_timer_running;
    bool is_demo_mode_enabled;
    bool is_profile_updated[BusyTimerProfileIdMax];
};
