#include "mqtt_profile_i.h"

#include <core/check.h>
#include <toolbox/value_index.h>

#define KEY_PROFILE_TYPE  "profile_type"
#define KEY_CUSTOM_CONFIG "custom_config"

#define KEY_CONFIG_SERVER_URL         "server_url"
#define KEY_CONFIG_CLIENT_CERT_TYPE   "client_cert_type"
#define KEY_CONFIG_IGNORE_SERVER_CERT "ignore_server_cert"

static const char* const mqtt_profile_type_values[MqttProfileTypeMax] = {
    [MqttProfileTypeDefault] = "default",
    [MqttProfileTypeCustom] = "custom",
};

static const char* const mqtt_profile_client_cert_type_values[MqttClientCertTypeMax] = {
    [MqttClientCertTypeDefault] = "default",
    [MqttClientCertTypeCustom] = "custom",
    [MqttClientCertTypeNone] = "none",
};

static void mqtt_profile_serialize_config(const MqttProfileConfig* config, cJSON* json) {
    cJSON_AddStringToObject(json, KEY_CONFIG_SERVER_URL, config->server_url);

    const MqttClientCertType client_cert_type = config->client_cert_type;
    furi_assert(client_cert_type < MqttClientCertTypeMax);

    cJSON_AddStringToObject(
        json, KEY_CONFIG_CLIENT_CERT_TYPE, mqtt_profile_client_cert_type_values[client_cert_type]);

    cJSON_AddBoolToObject(json, KEY_CONFIG_IGNORE_SERVER_CERT, config->is_ignore_server_cert);
}

static bool mqtt_profile_deserialize_type(MqttProfileType* type, cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsString(json)) {
            break;
        }

        const MqttProfileType found_type = value_index_string(
            cJSON_GetStringValue(json),
            mqtt_profile_type_values,
            COUNT_OF(mqtt_profile_type_values));

        if(found_type >= MqttProfileTypeMax) {
            break;
        }

        *type = found_type;

        success = true;
    } while(false);

    return success;
}

static bool mqtt_profile_deserialize_config(MqttProfileConfig* config, cJSON* json) {
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
            mqtt_profile_client_cert_type_values,
            COUNT_OF(mqtt_profile_client_cert_type_values));
        if(cert_type >= MqttClientCertTypeMax) {
            break;
        }

        config->client_cert_type = cert_type;

        item = cJSON_GetObjectItem(json, KEY_CONFIG_IGNORE_SERVER_CERT);
        if(!cJSON_IsBool(item)) {
            break;
        }

        config->is_ignore_server_cert = cJSON_IsTrue(item);

        success = true;
    } while(false);

    return success;
}

bool mqtt_profile_serialize_raw(const MqttProfile* profile, cJSON* json) {
    const MqttProfileType profile_type = profile->type;
    furi_assert(profile_type < MqttProfileTypeMax);

    cJSON_AddStringToObject(json, KEY_PROFILE_TYPE, mqtt_profile_type_values[profile_type]);

    if(profile_type == MqttProfileTypeCustom) {
        cJSON* custom_config_json = cJSON_AddObjectToObject(json, KEY_CUSTOM_CONFIG);
        mqtt_profile_serialize_config(&profile->custom_config, custom_config_json);
    }

    return true;
}

bool mqtt_profile_deserialize_raw(MqttProfile* profile, const cJSON* json) {
    bool success = false;

    do {
        if(!cJSON_IsObject(json)) {
            break;
        }

        cJSON* item;

        item = cJSON_GetObjectItem(json, KEY_PROFILE_TYPE);
        if(!mqtt_profile_deserialize_type(&profile->type, item)) {
            break;
        }

        if(profile->type == MqttProfileTypeCustom) {
            item = cJSON_GetObjectItem(json, KEY_CUSTOM_CONFIG);
            if(!mqtt_profile_deserialize_config(&profile->custom_config, item)) {
                break;
            }
        }

        success = true;
    } while(false);

    return success;
}

char* mqtt_profile_serialize(const MqttProfile* profile) {
    furi_check(profile);

    cJSON* json = cJSON_CreateObject();
    char* json_text = NULL;

    if(mqtt_profile_serialize_raw(profile, json)) {
        json_text = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return json_text;
}

bool mqtt_profile_deserialize(MqttProfile* profile, const char* json_text, size_t json_text_len) {
    furi_check(profile);

    cJSON* json = cJSON_ParseWithLength(json_text, json_text_len);
    const bool success = mqtt_profile_deserialize_raw(profile, json);

    cJSON_Delete(json);
    return success;
}

bool mqtt_profile_is_valid(const MqttProfile* profile) {
    furi_check(profile);
    return true;
}
