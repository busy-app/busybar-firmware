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

struct MqttClient {
    FuriPubSub* event_pubsub;
    struct mg_mgr mgr;
    struct mg_connection* conn;

    int profile_id;
    char* server_addr;
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
    enum {
        MqttClientMessageWifiStateChange,
        MqttClientMessageGetStatus,
        MqttClientMessageUnlink,
        MqttClientMessageRequestPin,
        MqttClientMessageGetSessionInfo,
        MqttClientMessageGetProfile,
        MqttClientMessageSetProfile,
    } type;
    FuriApiLock lock;
    union {
        MqttClientStatus* status;
        bool* bool_param;
        MqttClientProfile* profile;
        struct {
            FuriString* id;
            FuriString* email;
            FuriString* user_id;
        } session_info;

        WifiState wifi_state;
    };
} MqttClientMessage;

typedef struct {
    struct mg_str ca;
    struct mg_str name;
} MqttTlsCfg;

void mqtt_topics_subscribe(MqttClient* mqtt);
void mqtt_topics_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_mqtt_message* msg);
void mqtt_topics_on_close(MqttClient* mqtt);

void mqtt_http_api_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_mqtt_message* msg);

void mqtt_screen_streaming_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg);
void mqtt_screen_streaming_on_close(MqttClient* mqtt);

bool mqtt_tls_init(struct mg_connection* conn, const MqttTlsCfg* opts);
void mqtt_tls_free_ca(struct mg_connection* conn);
