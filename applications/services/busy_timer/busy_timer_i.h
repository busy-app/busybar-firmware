#pragma once

#include "busy_timer.h"
#include "settings/busy_timer_settings.h"
#include "settings/busy_timer_saved_state.h"

#include <furi.h>

#include <mqtt/mqtt.h>
#include <matter/matter.h>
#include <status_lights/status_lights.h>

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
    BusyTimerApiMessageTypeGetPreset,
    BusyTimerApiMessageTypeSetPreset,
    BusyTimerApiMessageTypeHandleMatter,

    BusyTimerApiMessageTypeMax,
} BusyTimerApiMessageType;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerSessionSource source;
} BusyTimerApiMessageStart;

typedef struct {
    int32_t time_minutes;
} BusyTimerApiMessageAddTime;

typedef struct {
    BusyTimerSnapshot* snapshot;
} BusyTimerApiMessageGetSnapshot;

typedef struct {
    BusyTimerSnapshot snapshot;
    BusyTimerSessionSource source;
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
    BusyTimerPreset* preset;
} BusyTimerApiMessageGetPreset;

typedef struct {
    BusyTimerProfileId profile_id;
    BusyTimerPreset preset;
} BusyTimerApiMessageSetPreset;

typedef struct {
    MatterSwitchState switch_state;
} BusyTimerApiMessageHandleMatter;

typedef union {
    BusyTimerApiMessageStart start;
    BusyTimerApiMessageAddTime add_time;
    BusyTimerApiMessageGetSnapshot get_snapshot;
    BusyTimerApiMessageSetSnapshot set_snapshot;
    BusyTimerApiMessageGetRunInfo get_run_info;
    BusyTimerApiMessageGetProfile get_profile;
    BusyTimerApiMessageSetProfile set_profile;
    BusyTimerApiMessageGetPreset get_preset;
    BusyTimerApiMessageSetPreset set_preset;
    BusyTimerApiMessageHandleMatter handle_matter;
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
    Matter* matter;
    MatterSwitchState matter_switch_state;
    StatusLights* status_lights;
    BusyTimerSnapshot last_known_snapshot;
    BusyTimerSettings settings[BusyTimerProfileIdMax];
    BusyTimerSavedState saved_state;
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
    uint32_t snapshot_update_count;
    uint32_t profile_update_count[BusyTimerProfileIdMax];
    bool is_timer_running;
    bool is_demo_mode_enabled;
    BusyTimerSessionSource session_source;
    BusyTimerProfileId active_profile_id;
};

// busy_timer.c

void busy_timer_apply_profile_settings(BusyTimer* instance, BusyTimerProfileId profile_id);

bool busy_timer_is_running(const BusyTimer* instance);

void busy_timer_start_internal(BusyTimer* instance);

void busy_timer_stop_internal(BusyTimer* instance);

void busy_timer_toggle_internal(BusyTimer* instance);

void busy_timer_skip_internal(BusyTimer* instance);

// busy_timer_api.c

void busy_timer_handle_matter(BusyTimer* instance, MatterSwitchState switch_state);

// busy_timer_smart_home.c

void busy_timer_smart_home_init(BusyTimer* instance);

void busy_timer_smart_home_handle_switch_state(BusyTimer* instance, MatterSwitchState switch_state);

// busy_timer_status_lights.c

void busy_timer_status_lights_init(BusyTimer* instance);

// busy_timer_util.c

void busy_timer_start_app(const BusyAppConfig* app_config);

void busy_timer_exit_app(void);
