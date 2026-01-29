#pragma once

#include <gui/gui.h>
#include <mqtt_client/mqtt_client.h>

typedef enum {
    MqttStreamingApiMessageTypeStart,
    MqttStreamingApiMessageTypeStop,
    MqttStreamingApiMessageTypeMax,
} MqttStreamingApiMessageType;

typedef struct {
    MqttStreamingApiMessageType type;
} MqttStreamingApiMessage;

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* frame_timer;
    FuriEventLoopTimer* timeout_timer;
    FuriMessageQueue* api_queue;
    MqttClient* mqtt;
    Gui* gui;
    uint8_t* frame_buf;
} MqttStreamingSrv;
