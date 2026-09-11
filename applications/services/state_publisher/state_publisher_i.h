#pragma once
#include "state_publisher.h"
#include <furi/furi.h>

#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <state.pb.h>

#include "screen_streamer.h"
#include <power/power_service/power.h>
#include <audio/audio.h>
#include <busy_timer/busy_timer.h>
#include <updater/updater.h>
#include <ble/ble.h>
#include <device_name/device_name.h>

#include <mlib/m-array.h>
#include <mlib/m-shared.h>

#define TAG "StPubSrv"

#define MAX_TRANSPORTS 16

typedef enum {
    StreamFlagMQTT = 1 << StatePublisherTransportClassMQTT,
    StreamFlagWebSocket = 1 << StatePublisherTransportClassWebSocket,
    StreamFlagBLE = 1 << StatePublisherTransportClassBLE,

    StreamFlagAll = StreamFlagMQTT | StreamFlagWebSocket | StreamFlagBLE
} StreamFlag;

void state_publisher_free_state_update(BSB_State_StateUpdate* update);

#define STATE_UPDATE_CLEAR(o) state_publisher_free_state_update(&(o))
#define STATE_UPDATE_OPLIST   M_OPEXTEND(M_EMPTY_OPLIST, CLEAR(STATE_UPDATE_CLEAR))
SHARED_PTR_DEF(SharedStateUpdate, BSB_State_StateUpdate, STATE_UPDATE_OPLIST);
ARRAY_DEF(SharedStateUpdateArray, SharedStateUpdate_t, SHARED_PTR_OPLIST(SharedStateUpdate));

#define MAX_SEQ_UPDATES 128

typedef struct Transport {
    bool valid;

    StreamFlag flags;
    uint32_t frame_interval_ms;

    RateLimiter limiter;
    /// Updates that must be sent all of: sequential array
    SharedStateUpdateArray_t seq_updates;
    /// State-like updates (only last one matters): array indexed by tag
    SharedStateUpdateArray_t state_updates;

    StatePublisherPublishCb cb;
    void* cb_context;
} Transport;

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* control_queue; // queue of Message
    FuriMessageQueue* publish_queue; // queue of PublishMessage
    FuriEventFlag* fetch_flags;

    FuriThreadId main_thread_id;

    FuriThread* fetch_thread;

    ScreenStreamer* screen_streamer_front;

    FuriEventLoopTimer* heartbeat_timer;
    FuriEventLoopTimer* rate_limiter_timer;

    Power* power;
    Audio* audio;
    Matter* matter;
    Updater* updater;
    BusyTimer* busy_timer;
    Gui* gui;
    Ble* ble;
    DeviceName* device_name;

    FuriState* state_brightness;
    FuriState* state_time_settings;
    FuriState* state_wifi;
    FuriState* state_switch_pos;

    FuriMutex* transports_mutex;
    Transport transports[MAX_TRANSPORTS];
};

typedef enum {
    MessageTypeTransportResumed,

    MessageTypesCount,
} MessageType;

typedef enum FetchBit {
    FetchBitPowerEvent,
    FetchBitAudioEvent,
    FetchBitMatterEvent,
    FetchBitUpdaterCheckEvent,
    FetchBitBusyTimer,
    FetchBitBusyTimerProfiles,
    FetchBitAutoupdateEvent,
    FetchBitBle,

    FetchBitMax
} FetchBit;

typedef enum {
    FetchFlagPowerEvent = (1 << FetchBitPowerEvent),
    FetchFlagAudioEvent = (1 << FetchBitAudioEvent),
    FetchFlagMatterEvent = (1 << FetchBitMatterEvent),
    FetchFlagUpdaterCheckEvent = (1 << FetchBitUpdaterCheckEvent),
    FetchFlagBusyTimer = (1 << FetchBitBusyTimer),
    FetchFlagBusyTimerProfiles = (1 << FetchBitBusyTimerProfiles),
    FetchFlagAutoupdateEvent = (1 << FetchBitAutoupdateEvent),
    FetchFlagBle = (1 << FetchBitBle),

    FetchFlagAll = ~(UINT32_MAX << FetchBitMax)
} FetchFlag;

typedef struct {
    MessageType type;
    union {
        UpdaterCheckState updater_check_state;
    };
} ControlMessage;

typedef struct {
    BSB_State_StateUpdate* data; // allocated on the heap
    StreamFlag stream_flags;
    bool heartbeat;
} PublishMessage;

void state_publisher_subscribe(StatePublisher* instance);

void state_publisher_store_power(StatePublisher* instance);
void state_publisher_store_audio(StatePublisher* instance);
void state_publisher_store_matter(StatePublisher* instance);
void state_publisher_store_update_check(StatePublisher* instance);
void state_publisher_store_busy_timer(StatePublisher* instance);
void state_publisher_store_busy_timer_profiles(StatePublisher* instance);
void state_publisher_store_autoupdate(StatePublisher* instance);
void state_publisher_store_ble(StatePublisher* instance);

BSB_State_State* state_publisher_collect_all(StatePublisher* instance);

void state_publisher_schedule_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags);

void state_publisher_store_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags);

void state_publisher_send_control_message(StatePublisher* instance, const ControlMessage* message);
