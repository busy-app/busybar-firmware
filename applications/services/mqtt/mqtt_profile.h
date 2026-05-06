#pragma once

#include <stddef.h>
#include <stdbool.h>

#define MQTT_PROFILE_SERVER_URL_LEN (64)

typedef enum {
    MqttProfileTypeDefault,
    MqttProfileTypeCustom,
    MqttProfileTypeMax,
} MqttProfileType;

typedef enum {
    MqttClientCertTypeDefault,
    MqttClientCertTypeCustom,
    MqttClientCertTypeNone,
    MqttClientCertTypeMax,
} MqttClientCertType;

typedef struct {
    char server_url[MQTT_PROFILE_SERVER_URL_LEN + 1];
    MqttClientCertType client_cert_type;
    bool is_ignore_server_cert;
} MqttProfileConfig;

typedef struct {
    MqttProfileType type;
    MqttProfileConfig custom_config;
} MqttProfile;

char* mqtt_profile_serialize(const MqttProfile* profile);

bool mqtt_profile_deserialize(MqttProfile* profile, const char* json_text, size_t json_text_len);

bool mqtt_profile_is_valid(const MqttProfile* profile);
