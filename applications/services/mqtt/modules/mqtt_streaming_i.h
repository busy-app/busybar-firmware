#pragma once

#include <gui/gui.h>
#include <mqtt/mqtt.h>
#include <state_publisher/state_publisher.h>

typedef enum {
    MqttStreamingApiMessageTypeStart,
    MqttStreamingApiMessageTypeStop,
    MqttStreamingApiMessageTypeMax,
} MqttStreamingApiMessageType;

typedef struct {
    MqttStreamingApiMessageType type;
    union {
        uint32_t expiry_interval;
    };
} MqttStreamingApiMessage;

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* frame_timer;
    FuriEventLoopTimer* timeout_timer;
    FuriMessageQueue* api_queue;
    Mqtt* mqtt;
    Gui* gui;
    StatePublisher* state_publisher;
    StatePublisherTransportHandle state_publisher_handle;
    uint8_t* frame_buf;
} MqttStreamingSrv;
