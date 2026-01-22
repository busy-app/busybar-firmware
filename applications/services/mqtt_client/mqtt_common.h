#pragma once

#define MQTT_URL_PREFIX     "mqtt://"
#define MQTT_URL_TLS_PREFIX "mqtts://"

typedef enum {
    MqttProfileIdDevelopment,
    MqttProfileIdProduction,
    MqttProfileIdLocal,
    MqttProfileIdCustom,
    MqttProfileIdMax,
} MqttProfileId;
