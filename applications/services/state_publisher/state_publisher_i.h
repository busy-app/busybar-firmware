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

#define TAG "StPubSrv"

#define MAX_TRANSPORTS 16

typedef enum {
    StreamFlagMQTT = 1 << StatePublisherTransportClassMQTT,
    StreamFlagWebSocket = 1 << StatePublisherTransportClassWebSocket,
    StreamFlagBLE = 1 << StatePublisherTransportClassBLE,

    StreamFlagAll = StreamFlagMQTT | StreamFlagWebSocket | StreamFlagBLE
} StreamFlag;

typedef struct Transport {
    bool valid;
    StreamFlag flags;
    uint32_t frame_interval_ms;
    StatePublisherPublishCb cb;
    void* cb_context;
} Transport;

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    FuriThreadId main_thread_id;

    ScreenStreamer* screen_streamer_front;

    FuriEventLoopTimer* heartbeat_timer;

    Power* power;
    Audio* audio;
    MatterSrv* matter;
    Updater* updater;
    BusyTimer* busy_timer;
    Gui* gui;

    FuriMutex* transports_mutex;
    Transport transports[MAX_TRANSPORTS];
};

typedef enum {
    MessageTypePublishUpdate,
    MessageTypePowerEvent,
    MessageTypeAudioEvent,
    MessageTypeMatterEvent,
    MessageTypeUpdaterCheckEvent,
    MessageTypeBusyTimer,

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

void state_publisher_schedule_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags);
void state_publisher_send_message(StatePublisher* instance, const Message* message);
