#include "mqtt_config_i.h"

#include <core/check.h>
#include <toolbox/value_index.h>

#include "mqtt_common.h"

#define KEY_CONFIG_SERVER_URL         "server_url"
#define KEY_CONFIG_CLIENT_CERT_TYPE   "client_cert_type"
#define KEY_CONFIG_IGNORE_SERVER_CERT "ignore_server_cert"

static const char* const mqtt_config_client_cert_types[MqttClientCertTypeMax] = {
    [MqttClientCertTypeDefault] = "default",
    [MqttClientCertTypeCustom] = "custom",
    [MqttClientCertTypeNone] = "none",
};

bool mqtt_config_serialize_raw(const MqttConfig* config, cJSON* json) {
    cJSON_AddStringToObject(json, KEY_CONFIG_SERVER_URL, config->server_url);

    const MqttClientCertType client_cert_type = config->client_cert_type;
    furi_assert(client_cert_type < MqttClientCertTypeMax);

    cJSON_AddStringToObject(
        json, KEY_CONFIG_CLIENT_CERT_TYPE, mqtt_config_client_cert_types[client_cert_type]);

    cJSON_AddBoolToObject(json, KEY_CONFIG_IGNORE_SERVER_CERT, config->ignore_server_cert);

    return true;
}

bool mqtt_config_deserialize_raw(MqttConfig* config, const cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_CONFIG_SERVER_URL);
        if(!cJSON_IsString(item)) {
            break;
        }

        strlcpy(config->server_url, cJSON_GetStringValue(item), sizeof(config->server_url));

        item = cJSON_GetObjectItem(json, KEY_CONFIG_CLIENT_CERT_TYPE);
        if(!cJSON_IsString(item)) {
            break;
        }

        const MqttClientCertType cert_type = value_index_string(
            cJSON_GetStringValue(item),
            mqtt_config_client_cert_types,
            COUNT_OF(mqtt_config_client_cert_types));
        if(cert_type >= MqttClientCertTypeMax) {
            break;
        }

        config->client_cert_type = cert_type;

        item = cJSON_GetObjectItem(json, KEY_CONFIG_IGNORE_SERVER_CERT);
        if(!cJSON_IsBool(item)) {
            break;
        }

        config->ignore_server_cert = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}

char* mqtt_config_serialize(const MqttConfig* config) {
    furi_check(config);

    cJSON* json = cJSON_CreateObject();
    char* json_text = NULL;

    if(mqtt_config_serialize_raw(config, json)) {
        json_text = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return json_text;
}

bool mqtt_config_deserialize(MqttConfig* config, const char* json_text, size_t json_text_len) {
    furi_check(config);

    cJSON* json = cJSON_ParseWithLength(json_text, json_text_len);
    const bool success = mqtt_config_deserialize_raw(config, json);

    cJSON_Delete(json);
    return success;
}

bool mqtt_config_is_valid(const MqttConfig* profile) {
    furi_check(profile);

    bool is_valid = false;

    do {
        const char* server_url = profile->server_url;
        if(strlen(server_url) == 0) {
            break;
        }

        if((strcmp(server_url, MQTT_CONFIG_SERVER_URL_DEFAULT) != 0) &&
           (strncmp(server_url, MQTT_URL_PREFIX, strlen(MQTT_URL_PREFIX)) != 0) &&
           (strncmp(server_url, MQTT_URL_TLS_PREFIX, strlen(MQTT_URL_TLS_PREFIX)) != 0)) {
            break;
        }

        if(profile->client_cert_type >= MqttClientCertTypeMax) {
            break;
        }

        is_valid = true;
    } while(false);

    return is_valid;
}
