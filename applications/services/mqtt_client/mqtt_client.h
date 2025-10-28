#pragma once

#include <furi.h>

#define RECORD_MQTT "MQTT"

#define MQTT_LINK_PIN_LEN (4)

typedef enum {
    MqttClientStatusError, // Clent certificates missing
    MqttClientStatusNotConnected, // Not connected to MQTT broker
    MqttClientStatusConnectedNotLinked, // Connected to MQTT broker, not linked
    MqttClientStatusConnectedLinked, // Connected to MQTT broker, linked
} MqttClientStatus;

typedef struct {
    enum {
        MqttClientEventStatusChange,
        MqttClientEventLinkPin,
        MqttClientEventLinkDone,
    } type;
    union {
        struct {
            const char* pin;
            uint32_t expires_at;
        } link;
        MqttClientStatus status;
    };
} MqttClientEvent;

typedef struct MqttClient MqttClient;

FuriPubSub* mqtt_client_get_pubsub(MqttClient* mqtt);

MqttClientStatus mqtt_client_get_status(MqttClient* mqtt);
bool mqtt_client_request_link_pin(MqttClient* mqtt);
void mqtt_client_unlink(MqttClient* mqtt);
void mqtt_client_get_session_id(MqttClient* mqtt, FuriString* id);
void mqtt_client_get_session_email(MqttClient* mqtt, FuriString* id);
