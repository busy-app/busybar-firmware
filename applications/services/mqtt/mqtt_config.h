#pragma once

#include <stddef.h>
#include <stdbool.h>

#define MQTT_CONFIG_SERVER_URL_LEN (64)

typedef enum {
    MqttClientCertTypeDefault,
    MqttClientCertTypeCustom,
    MqttClientCertTypeNone,
    MqttClientCertTypeMax,
} MqttClientCertType;

typedef struct {
    char server_url[MQTT_CONFIG_SERVER_URL_LEN + 1];
    MqttClientCertType client_cert_type;
    bool ignore_server_cert;
} MqttConfig;

char* mqtt_config_serialize(const MqttConfig* config);

bool mqtt_config_deserialize(MqttConfig* config, const char* json_text, size_t json_text_len);

bool mqtt_config_is_valid(const MqttConfig* config);
