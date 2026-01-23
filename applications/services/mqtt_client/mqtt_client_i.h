#pragma once

#include "mqtt_client.h"

#include <furi.h>
#include <api_lock.h>

#include <mongoose.h>

#include <wifi/wifi.h>

#include "settings/mqtt_settings.h"
#include "settings/mqtt_saved_state.h"

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

    struct mg_timer reconnect_delay_timer;
    uint32_t reconnect_delay;

    unsigned long wakeup_conn_id;

    struct mg_timer ping_timer;
    bool ping_enabled;

    MqttClientStatus status;
    bool is_wifi_up;
    bool fast_reconnect;

    char* ca_bundle;

    FuriString* device_serial;

    struct mg_timer screen_stream_timer;
    FuriString* screen_stream_topic;
    char* screen_stream_buf;

    MqttSettings settings;
    MqttSavedState saved_state;
};

typedef enum {
    MqttApiMessageTypeGetStatus,
    MqttApiMessageTypeUnlink,
    MqttApiMessageTypeRequestPin,
    MqttApiMessageTypeGetSessionInfo,
    MqttApiMessageTypeGetProfile,
    MqttApiMessageTypeSetProfile,
    MqttApiMessageTypePublish,
    MqttApiMessageTypeWifiState,
    MqttApiMessageTypeMax,
} MqttApiMessageType;

typedef struct {
    MqttClientStatus* status;
} MqttApiMessageGetStatus;

typedef struct {
    bool* is_success;
} MqttApiMessageRequestPin;

typedef struct {
    MqttProfileId* profile_id;
    FuriString* custom_url;
} MqttApiMessageGetProfile;

typedef struct {
    MqttProfileId profile_id;
    const char* custom_url;
} MqttApiMessageSetProfile;

typedef struct {
    FuriString* session_id;
    FuriString* user_id;
    FuriString* email;
} MqttApiMessageGetSessionInfo;

typedef struct {
    const char* topic;
    const void* data;
    size_t data_size;
    MqttQos qos;
} MqttApiMessagePublish;

typedef struct {
    WifiState state;
} MqttApiMessageWifiState;

typedef union {
    MqttApiMessageGetStatus get_status;
    MqttApiMessageRequestPin request_pin;
    MqttApiMessageGetProfile get_profile;
    MqttApiMessageSetProfile set_profile;
    MqttApiMessageGetSessionInfo get_session_info;
    MqttApiMessagePublish publish;
    MqttApiMessageWifiState wifi_state;
} MqttApiMessageData;

typedef struct {
    MqttApiMessageType type;
    MqttApiMessageData data;
    FuriApiLock lock;
} MqttApiMessage;

void mqtt_topics_subscribe(MqttClient* mqtt);

void mqtt_topics_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg);

void mqtt_topics_on_close(MqttClient* mqtt);

void mqtt_http_api_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg);

void mqtt_screen_streaming_on_message(
    MqttClient* mqtt,
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg);

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
    const FuriString* topic_str,
    const struct mg_mqtt_message* msg);
