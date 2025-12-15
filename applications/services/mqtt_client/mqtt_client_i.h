#pragma once

#include <furi.h>
#include <toolbox/api_lock.h>
#include <mongoose.h>
#include <wifi/wifi.h>
#include "mqtt_client.h"

#define MQTT_RECONNECT_DELAY_MIN (2000)
#define MQTT_RECONNECT_DELAY_MAX (60000)
#define MQTT_POLL_PERIOD         (100)
#define MQTT_QOS                 (2)
#define MQTT_API_VERSION         "v1"

#define MQTT_DEVICE_ROOT_TOPIC "devices"
#define MQTT_API_ROOT_TOPIC    "sessions"

#define MQTT_BUSY_TIMER_SNAPSHOT_TOPIC "busy/snapshot"

struct MqttClient {
    FuriPubSub* event_pubsub;
    struct mg_mgr mgr;
    struct mg_connection* conn;

    int profile_id;
    FuriString* server_addr;
    bool use_tls;

    struct mg_timer reconnect_delay_timer;
    uint32_t reconnect_delay;

    unsigned long wakeup_conn_id;

    struct mg_timer ping_timer;
    bool ping_enabled;

    MqttClientStatus status;
    bool is_wifi_up;
    bool is_linked;
    bool fast_reconnect;

    char* ca_bundle;

    FuriString* device_serial;
    FuriString* client_id;
    FuriString* session_id;
    FuriString* link_token;

    struct mg_timer screen_stream_timer;
    FuriString* screen_stream_topic;
    char* screen_stream_buf;
};

typedef struct {
    const char* topic;
    const void* data;
    size_t data_size;
    MqttQos qos;
} MqttClientPublish;

typedef struct {
    enum {
        MqttClientMessageWifiStateChange,
        MqttClientMessageGetStatus,
        MqttClientMessageUnlink,
        MqttClientMessageRequestPin,
        MqttClientMessageGetSessionInfo,
        MqttClientMessageGetProfile,
        MqttClientMessageSetProfile,
        MqttClientMessagePublish,
    } type;
    FuriApiLock lock;
    union {
        MqttClientStatus* status;
        bool* bool_param;
        struct {
            MqttClientProfile* id;
            FuriString* custom_url;
        } profile;
        struct {
            FuriString* id;
            FuriString* email;
            FuriString* user_id;
        } session_info;

        WifiState wifi_state;
        MqttClientPublish publish;
    };
} MqttClientMessage;

void mqtt_topics_subscribe(MqttClient* mqtt);
void mqtt_topics_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_mqtt_message* msg);
void mqtt_topics_on_close(MqttClient* mqtt);

void mqtt_http_api_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_mqtt_message* msg);

void mqtt_screen_streaming_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg);
void mqtt_screen_streaming_on_close(MqttClient* mqtt);

bool mqtt_tls_init(
    struct mg_connection* conn,
    struct mg_str name,
    struct mg_str ca,
    bool custom_certs);
void mqtt_tls_free_ca(struct mg_connection* conn);

// BusyTimer api
void mqtt_busy_timer_init(MqttClient* mqtt);

void mqtt_busy_timer_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg);
