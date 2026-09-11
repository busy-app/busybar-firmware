#pragma once

#include "mqtt.h"
#include "mqtt_config.h"

#include <furi.h>
#include <api_lock.h>

#include <m-i-list.h>

#include <mongoose.h>

#include <wifi/wifi.h>

#include "settings/mqtt_settings.h"
#include "settings/mqtt_saved_state.h"

#define TAG "Mqtt"

#define MQTT_RECONNECT_DELAY_MIN (2000UL)
#define MQTT_RECONNECT_DELAY_MAX (60000UL)
#define MQTT_POLL_PERIOD_MS      (1000UL)
#define MQTT_API_VERSION         "v1"

#define MQTT_ROOT_TOPIC_DEVICE  "devices"
#define MQTT_ROOT_TOPIC_SESSION "sessions"

#define MQTT_TOPIC_PRESENCE           "presence"
#define MQTT_TOPIC_UNLINK_FROM_DEVICE "unlink"
#define MQTT_TOPIC_UNLINK_FROM_CLOUD  "gone"

#define MQTT_TOPIC_LINK_REQUEST "link/request"
#define MQTT_TOPIC_LINK_PIN     "link/otp"
#define MQTT_TOPIC_LINK_DONE    "link/token"

#define MQTT_DIRECTION_UP   "up"
#define MQTT_DIRECTION_DOWN "down"

// NOTE: MqttMessage is an opaque alias for mg_mqtt_message.
#define TO_RAW_MESSAGE(msg)  ((struct mg_mqtt_message*)(msg))
#define TO_MQTT_MESSAGE(msg) ((MqttMessage*)(msg))

// Source: https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031
typedef enum {
    MqttReasonCodeSuccess = 0x0,
    MqttReasonCodeGrantedQoS0 = MqttReasonCodeSuccess,
    MqttReasonCodeGrantedQoS1,
    MqttReasonCodeGrantedQoS2,
    // Reason codes 0x4 ... 0x19 omitted for clarity
    MqttReasonCodeUnspecifiedError = 0x80,
    // Reason codes 0x81 ... 0x86 omitted for clarity
    MqttReasonCodeNotAuthorized = 0x87,
    // Reason codes 0x88 ... 0xA2 omitted for clarity
} MqttReasonCode;

typedef enum {
    MqttScopeDevice,
    MqttScopeSession,
    MqttScopeMax,
} MqttScope;

typedef enum {
    MqttPropertyValueTypeInteger,
    MqttPropertyValueTypeString,
    MqttPropertyValueTypeMax,
} MqttPropertyValueType;

typedef struct {
    uint8_t raw_id;
    MqttPropertyValueType value_type;
} MqttPropertyDesc;

struct MqttSubscription {
    FuriString* topic;
    MqttScope scope;
    MqttQos qos;
    MqttSubscriptionCallback callback;
    void* callback_context;
    ILIST_INTERFACE(MqttSubscriptionList, MqttSubscription);
};

ILIST_DEF(MqttSubscriptionList, MqttSubscription, M_POD_OPLIST)

struct Mqtt {
    struct mg_mgr mgr;
    struct mg_connection* conn;
    struct mg_timer reconnect_timer;
    struct mg_timer ping_timer;

    FuriPubSub* event_pubsub;
    FuriString* device_serial;

    MqttSubscriptionList_t subscriptions;

    unsigned long api_connection_id;
    uint32_t reconnect_delay_ms;

    MqttSettings settings;
    MqttSavedState saved_state;
    MqttStatus status;

    bool is_wifi_up;
    bool is_ping_enabled;
    bool should_reconnect_now;
};

typedef enum {
    MqttApiMessageTypeGetStatus,
    MqttApiMessageTypeUnlink,
    MqttApiMessageTypeRequestPin,
    MqttApiMessageTypeGetSessionInfo,
    MqttApiMessageTypeGetConfig,
    MqttApiMessageTypeSetConfig,
    MqttApiMessageTypePublish,
    MqttApiMessageTypePublishDevice,
    MqttApiMessageTypeSubscribe,
    MqttApiMessageTypeUnsubscribe,
    MqttApiMessageTypeWifiState,
    MqttApiMessageTypeMax,
} MqttApiMessageType;

typedef struct {
    MqttStatus* status;
} MqttApiMessageGetStatus;

typedef struct {
    bool* is_success;
} MqttApiMessageRequestPin;

typedef struct {
    MqttConfig* config;
} MqttApiMessageGetConfig;

typedef struct {
    const MqttConfig* config;
    bool* is_success;
} MqttApiMessageSetConfig;

typedef struct {
    FuriString* session_id;
    FuriString* user_id;
    FuriString* email;
    bool* is_valid;
} MqttApiMessageGetSessionInfo;

typedef struct {
    const char* topic;
    const void* data;
    size_t data_size;
    const MqttProperty* props;
    uint32_t props_count;
    MqttQos qos;
    bool* is_success;
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
    MqttApiMessageGetConfig get_config;
    MqttApiMessageSetConfig set_config;
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

void mqtt_api_init(Mqtt* instance);

void mqtt_account_init(Mqtt* instance);

void mqtt_account_unlink(Mqtt* instance);

void mqtt_connection_open(Mqtt* instance);

void mqtt_connection_close(Mqtt* instance, bool reconnect_now);

void mqtt_reset_saved_state(Mqtt* instance);

MqttSubscription* mqtt_subscribe_internal(
    Mqtt* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context);

void mqtt_unsubscribe_internal(Mqtt* instance, MqttSubscription* subscription);

void mqtt_subscription_activate(Mqtt* instance, const MqttSubscription* subscription);

bool mqtt_publish_internal(
    Mqtt* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const MqttProperty* props,
    uint32_t props_count);

void mqtt_property_to_raw(const MqttProperty* property, struct mg_mqtt_prop* raw_property);

void mqtt_make_topic_path(
    Mqtt* instance,
    MqttScope scope,
    const char* dir,
    const char* topic,
    FuriString* out);
