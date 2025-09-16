#pragma once

#include <furi.h>
#include <toolbox/api_lock.h>
#include <mongoose.h>
#include "mqtt_client.h"

#define MQTT_SERVER_ADDR     "mqtts://mqtt.cloud.dev.busy.app:8883"
#define MQTT_RECONNECT_DELAY (2000)
#define MQTT_QOS             (2)
#define MQTT_API_VERSION     "v1"

#define MQTT_DEVICE_ROOT_TOPIC "devices"
#define MQTT_API_ROOT_TOPIC    "sessions"

struct MqttClient {
    FuriPubSub* event_pubsub;
    struct mg_mgr mgr;
    struct mg_timer reconnect_delay_timer;
    struct mg_connection* conn;
    unsigned long wakeup_conn_id;
    MqttClientStatus status;
    bool is_linked;
    char* ca_bundle;
    char* device_cert;
    char* device_key;
    FuriString* device_serial;
    FuriString* client_id;
    FuriString* session_id;
    FuriString* link_token;
};

typedef struct {
    enum {
        MqttClientMessageGetStatus,
        MqttClientMessageUnlink,
        MqttClientMessageRequestPin,
        MqttClientMessageGetSessionId,
    } type;
    FuriApiLock lock;
    union {
        MqttClientStatus* status;
        bool* bool_param;
        FuriString* str_param;
    };
} MqttClientMessage;

void mqtt_api_subscribe(MqttClient* mqtt);
void mqtt_api_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_str* message);
