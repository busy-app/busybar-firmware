#pragma once

#include <mongoose.h>

#include <mqtt/mqtt.h>

typedef enum {
    MqttHttpProxyMethodIdGet,
    MqttHttpProxyMethodIdPost,
    MqttHttpProxyMethodIdPut,
    MqttHttpProxyMethodIdDelete,
    MqttHttpProxyMethodIdMax,
} MqttHttpProxyMethodId;

typedef struct {
    const char* name;
    MqttHttpProxyMethodId id;
} MqttHttpProxyBlocklistEntry;

typedef struct {
    Mqtt* mqtt;
    struct mg_mgr mgr;
    unsigned long api_connection_id;
} MqttHttpProxySrv;

typedef struct {
    Mqtt* mqtt;
    FuriString* response_topic;
    FuriString* correlation_data;
    uint8_t* data;
    size_t data_size;
    uint32_t start_tick;
} MqttHttpProxyRequest;
