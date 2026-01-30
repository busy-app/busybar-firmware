#pragma once

#include <gui/gui.h>
#include <mqtt/mqtt.h>

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
    uint8_t* frame_buf;
} MqttStreamingSrv;
