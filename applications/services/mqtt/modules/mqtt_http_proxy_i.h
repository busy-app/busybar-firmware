#pragma once

#include <mongoose.h>

#include <mqtt/mqtt_client.h>

typedef enum {
    MqttHttpProxyMethodIdGet,
    MqttHttpProxyMethodIdPost,
    MqttHttpProxyMethodIdDelete,
    MqttHttpProxyMethodIdMax,
} MqttHttpProxyMethodId;

typedef struct {
    const char* name;
    MqttHttpProxyMethodId id;
} MqttHttpProxyBlocklistEntry;

typedef struct {
    MqttClient* mqtt;
    struct mg_mgr mgr;
    struct mg_timer timer;
    unsigned long api_connection_id;
} MqttHttpProxySrv;

typedef struct {
    MqttClient* mqtt;
    FuriString* response_topic;
    FuriString* correlation_data;
    uint8_t* data;
    size_t data_size;
    uint32_t poll_cnt;
} MqttHttpProxyRequest;
