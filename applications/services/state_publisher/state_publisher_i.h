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
ARRAY_DEF(StateUpdateArray, SharedStateUpdate_t, SHARED_PTR_OPLIST(SharedStateUpdate));

#define MAX_SEQ_UPDATES 128

typedef struct Transport {
    bool valid;

    StreamFlag flags;
    uint32_t frame_interval_ms;

    RateLimiter limiter;
    /// Updates that must be sent all of: sequential array
    StateUpdateArray_t seq_updates;
    /// State-like updates (only last one matters): array indexed by tag
    StateUpdateArray_t state_updates;

    StatePublisherPublishCb cb;
    void* cb_context;
} Transport;

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    FuriThreadId main_thread_id;

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

    FuriMutex* transports_mutex;
    Transport transports[MAX_TRANSPORTS];
};

typedef enum {
    MessageTypePublishUpdate,
    MessageTypeTransportResumed,
    MessageTypePowerEvent,
    MessageTypeAudioEvent,
    MessageTypeMatterEvent,
    MessageTypeUpdaterCheckEvent,
    MessageTypeBusyTimer,
    MessageTypeBusyTimerProfiles,
    MessageTypeAutoupdateEvent,
    MessageTypeBle,

    MessageTypesCount,
} MessageType;

typedef struct {
    MessageType type;
    union {
        struct {
            BSB_State_StateUpdate* data; // allocated on the heap
            StreamFlag stream_flags;
        } update;
        UpdaterCheckState updater_check_state;
    };
} Message;

void state_publisher_subscribe(StatePublisher* instance);

void state_publisher_publish_power(StatePublisher* instance);
void state_publisher_publish_audio(StatePublisher* instance);
void state_publisher_publish_matter(StatePublisher* instance);
void state_publisher_publish_update_check(
    StatePublisher* instance,
    const UpdaterCheckState* check_state);
void state_publisher_publish_busy_timer(StatePublisher* instance);
void state_publisher_publish_busy_timer_profiles(StatePublisher* instance);
void state_publisher_publish_autoupdate(StatePublisher* instance);
void state_publisher_publish_ble(StatePublisher* instance);

void state_publisher_schedule_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags);
void state_publisher_send_message(StatePublisher* instance, const Message* message);
