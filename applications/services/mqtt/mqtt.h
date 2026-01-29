#pragma once

#include "mqtt_common.h"

#include <core/pubsub.h>
#include <core/string.h>

#define RECORD_MQTT "mqtt"

#define MQTT_LINK_PIN_LEN (4)

typedef struct Mqtt Mqtt;

typedef struct MqttMessage MqttMessage;

typedef struct MqttSubscription MqttSubscription;

typedef void (*MqttSubscriptionCallback)(const MqttMessage* message, void* context);

typedef enum {
    MqttPropertyTypeResponseTopic,
    MqttPropertyTypeCorrelationData,
    /* Add more property types as needed */
    MqttPropertyTypeMax,
} MqttPropertyType;

typedef struct {
    MqttPropertyType type;
    union {
        int32_t number;
        const char* string;
    } value;
} MqttProperty;

typedef enum {
    MqttEventTypeStatusChanged,
    MqttEventTypeLinkPinReceived,
    MqttEventTypeLinkDone,
    MqttEventTypeMax,
} MqttEventType;

typedef struct {
    const char* pin;
    uint32_t expires_at;
} MqttEventLinkPinReceived;

typedef enum {
    MqttStatusError, // Clent certificates missing
    MqttStatusNotConnected, // Not connected to MQTT broker
    MqttStatusConnectedNotLinked, // Connected to MQTT broker, not linked
    MqttStatusConnectedLinked, // Connected to MQTT broker, linked
} MqttStatus;

typedef struct {
    MqttStatus status;
} MqttEventStatusChanged;

typedef struct {
    MqttEventType type;
    union {
        MqttEventStatusChanged status_changed;
        MqttEventLinkPinReceived link_pin_received;
    };
} MqttEvent;

typedef enum {
    MqttQosAtMostOnce = 0,
    MqttQosAtLeastOnce = 1,
    MqttQosExactlyOnce = 2,
    MqttQosMax,
} MqttQos;

FuriPubSub* mqtt_get_pubsub(Mqtt* mqtt);

MqttStatus mqtt_get_status(Mqtt* mqtt);

bool mqtt_request_link_pin(Mqtt* mqtt);

void mqtt_unlink(Mqtt* mqtt);

void mqtt_get_session_info(Mqtt* mqtt, FuriString* id, FuriString* email, FuriString* user_id);

MqttProfileId mqtt_get_profile(Mqtt* mqtt, FuriString* custom_url);

void mqtt_set_profile(Mqtt* mqtt, MqttProfileId profile_id, const char* custom_url);

void mqtt_publish(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size);

void mqtt_publish_ex(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const MqttProperty* props,
    uint32_t props_count);

MqttSubscription* mqtt_subscribe(
    Mqtt* instance,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context);

void mqtt_unsubscribe(Mqtt* instance, MqttSubscription* subscription);

const void* mqtt_message_get_data(const MqttMessage* message, size_t* data_size);

bool mqtt_message_get_string_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    FuriString* value);
