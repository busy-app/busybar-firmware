#pragma once

#include "mqtt_client.h"

#include <furi.h>
#include <api_lock.h>

#include <m-i-list.h>

#include <mongoose.h>

#include <wifi/wifi.h>

#include "settings/mqtt_settings.h"
#include "settings/mqtt_saved_state.h"

#define MQTT_RECONNECT_DELAY_MIN (2000)
#define MQTT_RECONNECT_DELAY_MAX (60000)
#define MQTT_POLL_PERIOD         (100)
#define MQTT_QOS                 (2)
#define MQTT_API_VERSION         "v1"

#define MQTT_DEVICE_ROOT_TOPIC  "devices"
#define MQTT_SESSION_ROOT_TOPIC "sessions"

// NOTE: MqttMessage is an opaque alias for mg_mqtt_message.
#define TO_RAW_MESSAGE(msg)  ((const struct mg_mqtt_message*)(msg))
#define TO_MQTT_MESSAGE(msg) ((const MqttMessage*)(msg))

typedef enum {
    MqttScopeDevice,
    MqttScopeSession,
    MqttScopeMax,
} MqttScope;

struct MqttSubscription {
    FuriString* topic;
    MqttScope scope;
    MqttQos qos;
    MqttSubscriptionCallback callback;
    void* callback_context;
    ILIST_INTERFACE(MqttSubscriptionList, MqttSubscription);
};

ILIST_DEF(MqttSubscriptionList, MqttSubscription, M_POD_OPLIST)

struct MqttClient {
    FuriPubSub* event_pubsub;
    struct mg_mgr mgr;
    struct mg_connection* conn;

    struct mg_timer reconnect_delay_timer;
    uint32_t reconnect_delay;

    unsigned long api_connection_id;

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

    MqttSubscriptionList_t subscriptions;

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
    MqttApiMessageTypeSubscribe,
    MqttApiMessageTypeUnsubscribe,
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
    const char* topic;
    MqttSubscriptionCallback callback;
    void* callback_context;
    MqttQos qos;
    MqttSubscription** subscription;
} MqttApiMessageSubscribe;

typedef struct {
    MqttSubscription* subscription;
} MqttApiMessageUnsubscribe;

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
    MqttApiMessageSubscribe subscribe;
    MqttApiMessageUnsubscribe unsubscribe;
    MqttApiMessageWifiState wifi_state;
} MqttApiMessageData;

typedef struct {
    MqttApiMessageType type;
    MqttApiMessageData data;
    FuriApiLock lock;
} MqttApiMessage;

void mqtt_api_init(MqttClient* instance);

void mqtt_connection_open(MqttClient* instance);

void mqtt_reset_saved_state(MqttClient* instance);

const char* mqtt_get_server_url(const MqttClient* instance);

void mqtt_set_status(MqttClient* instance, MqttClientStatus status);

bool mqtt_is_tls_enabled(const MqttClient* instance);

void mqtt_make_topic_path(
    MqttClient* instance,
    MqttScope scope,
    const char* dir,
    const char* topic,
    FuriString* out);

MqttSubscription* mqtt_subscribe_internal(
    MqttClient* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context);

void mqtt_unsubscribe_internal(MqttClient* instance, MqttSubscription* subscription);

uint16_t mqtt_publish_internal(
    MqttClient* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const struct mg_mqtt_prop* props,
    uint32_t props_count);

bool mqtt_tls_init(
    struct mg_connection* conn,
    struct mg_str name,
    struct mg_str ca,
    bool custom_certs);

void mqtt_tls_free_ca(struct mg_connection* conn);
