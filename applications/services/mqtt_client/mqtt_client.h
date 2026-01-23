#pragma once

#include "mqtt_common.h"

#include <core/pubsub.h>
#include <core/string.h>

#define RECORD_MQTT "mqtt"

#define MQTT_LINK_PIN_LEN (4)

typedef struct MqttClient MqttClient;

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
    MqttClientStatusError, // Clent certificates missing
    MqttClientStatusNotConnected, // Not connected to MQTT broker
    MqttClientStatusConnectedNotLinked, // Connected to MQTT broker, not linked
    MqttClientStatusConnectedLinked, // Connected to MQTT broker, linked
} MqttClientStatus;

typedef struct {
    MqttClientStatus status;
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

FuriPubSub* mqtt_client_get_pubsub(MqttClient* mqtt);

MqttClientStatus mqtt_client_get_status(MqttClient* mqtt);

bool mqtt_client_request_link_pin(MqttClient* mqtt);

void mqtt_client_unlink(MqttClient* mqtt);

void mqtt_client_get_session_info(
    MqttClient* mqtt,
    FuriString* id,
    FuriString* email,
    FuriString* user_id);

MqttProfileId mqtt_client_get_profile(MqttClient* mqtt, FuriString* custom_url);

void mqtt_client_set_profile(MqttClient* mqtt, MqttProfileId profile_id, const char* custom_url);

void mqtt_client_publish(
    MqttClient* mqtt,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size);
