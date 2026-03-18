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
        struct {
            uint32_t expiry_interval;
            FuriString* response_topic;
        };
    };
} MqttStreamingApiMessage;

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timeout_timer;
    FuriMessageQueue* api_queue;
    Mqtt* mqtt;
    Gui* gui;
    StatePublisher* state_publisher;
    StatePublisherTransportHandle state_publisher_handle;
    FuriString* response_topic;
} MqttStreamingSrv;
