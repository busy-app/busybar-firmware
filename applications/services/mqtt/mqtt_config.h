/**
 * @file mqtt_config.h
 * @brief Mqtt backend configuration strcuture types and APIs.
 */
#pragma once

#include <stddef.h>
#include <stdbool.h>

/** Maximum server URL length (not counting the termination byte) */
#define MQTT_CONFIG_SERVER_URL_LEN     (64)
/** Special value for the default server URL (determined by firmware) */
#define MQTT_CONFIG_SERVER_URL_DEFAULT "default"

/**
 * @brief Enumeration of supported client certificate types.
 *
 * @note To use custom certificates, put the files into the following locations:
 *       - /ext/apps_assets/mqtt/device.crt : custom certificate
 *       - /ext/apps_assets/mqtt/device.key : private key
 */
typedef enum {
    MqttClientCertTypeDefault, /**< Built-in certificate */
    MqttClientCertTypeCustom, /**< User-specified certificate */
    MqttClientCertTypeNone, /**< No certificate */
    MqttClientCertTypeMax, /**< Special value, internal use */
} MqttClientCertType;

/**
 * @brief Mqtt backend configuration structure.
 */
typedef struct {
    char server_url[MQTT_CONFIG_SERVER_URL_LEN + 1]; /**< MQTT server URL to connect to */
    MqttClientCertType client_cert_type; /**< Client certificate type to use */
    bool ignore_server_cert; /**< @c true to ignore the server certificate (e.g. when self-signing) */
} MqttConfig;

/**
 * @brief Convert the MqttConfig object to a JSON representation.
 *
 * @note The calling code is responsible for deleting
 *       the returned value when it is done with it
 *
 * @param[in] config pointer to the config to be serialized
 * @returns pointer to a 0-terminated string on success, @c NULL on failure
 */
char* mqtt_config_serialize(const MqttConfig* config);

/**
 * @brief Construct a MqttConfig object from its JSON representation.
 *
 * @note Data pointed to by @p json_text may either be 0-terminated or not,
 *       provided that @p json_text_len is correct.
 *
 * @param[out] config pointer to the config to be constructed (must be allocated)
 * @param[in] json_text pointer to a character array containing the JSON representation
 * @param[in] json_text_len length of the @p json_text
 */
bool mqtt_config_deserialize(MqttConfig* config, const char* json_text, size_t json_text_len);

/**
 * @brief Check whether a MqttConfig object has valid field values.
 *
 * @param[in] config pointer to the config to be checked
 * @returns @c true if the config is valid, @c false otherwise
 */
bool mqtt_config_is_valid(const MqttConfig* config);
