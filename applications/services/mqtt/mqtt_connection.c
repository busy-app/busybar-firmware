#include "mqtt_i.h"

#include <storage/storage.h>
#include <busy_timer/time_macros.h>
#include <version/version.h>
#include <web_server/web_server.h>

#include "mqtt_common.h"

#define MQTT_VERSION     (5)
#define MQTT_PING_PERIOD M_TO_MS(10)

#define CERT_FILE_CA_BUNDLE EXT_PATH("apps_assets/ca/cacert.pem")

#define STATUS_ONLINE  "\"status\":\"online\""
#define STATUS_OFFLINE "\"status\":\"offline\""

#define MQTT_SERVER_URL_DEFAULT MQTT_URL_TLS_PREFIX "mqtt.busy.app:8883"

static void mqtt_ping_timer_callback(void* data) {
    furi_assert(data);
    Mqtt* mqtt = data;

    if(mqtt->conn) {
        FURI_LOG_D(TAG, "-> PING");
        mg_mqtt_ping(mqtt->conn);
    }
}

static void mqtt_start_ping_timer(Mqtt* instance) {
    if(!instance->is_ping_enabled) {
        mg_timer_init(
            &instance->mgr.timers,
            &instance->ping_timer,
            MQTT_PING_PERIOD,
            MG_TIMER_REPEAT,
            mqtt_ping_timer_callback,
            instance);

        instance->is_ping_enabled = true;
    }
}

static void mqtt_stop_ping_timer(Mqtt* instance) {
    if(instance->is_ping_enabled) {
        mg_timer_free(&instance->mgr.timers, &instance->ping_timer);
        instance->is_ping_enabled = false;
    }
}

static void mqtt_reconnect_callback(void* data) {
    furi_assert(data);
    Mqtt* instance = data;

    mqtt_connection_open(instance);
}

static void mqtt_start_reconnect_timer(Mqtt* instance) {
    mg_timer_init(
        &instance->mgr.timers,
        &instance->reconnect_timer,
        instance->reconnect_delay_ms,
        MG_TIMER_ONCE,
        mqtt_reconnect_callback,
        instance);

    instance->reconnect_delay_ms =
        MIN(instance->reconnect_delay_ms * 2UL, MQTT_RECONNECT_DELAY_MAX);
}

static void mqtt_stop_reconnect_timer(Mqtt* instance) {
    mg_timer_free(&instance->mgr.timers, &instance->reconnect_timer);
}

static void mqtt_set_status(Mqtt* instance, MqttStatus status) {
    if(status != instance->status) {
        instance->status = status;

        MqttEvent event = {
            .type = MqttEventTypeStatusChanged,
            .status_changed =
                {
                    .status = status,
                },
        };

        furi_pubsub_publish(instance->event_pubsub, &event);
    }
}

static const char* mqtt_get_server_url(const Mqtt* instance) {
    const MqttConfig* config = &instance->settings.config;
    if(strcmp(config->server_url, MQTT_CONFIG_SERVER_URL_DEFAULT) == 0) {
        return MQTT_SERVER_URL_DEFAULT;
    } else {
        return config->server_url;
    }
}

static bool mqtt_load_ca_bundle(Mqtt* instance) {
    furi_assert(instance->ca_bundle == NULL);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, CERT_FILE_CA_BUNDLE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "CA bundle file error: %s", storage_file_get_error_desc(file));
            break;
        }

        const uint64_t file_size = storage_file_size(file);
        instance->ca_bundle = malloc(file_size);

        if(storage_file_read(file, instance->ca_bundle, file_size) != file_size) {
            FURI_LOG_E(TAG, "CA bundle file read error");
            break;
        }

        success = true;
    } while(0);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return success;
}

static void mqtt_connect_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const void* event_data) {
    UNUSED(event_data);

    bool success = false;

    do {
        const char* server_url = mqtt_get_server_url(instance);

        if(!mg_url_is_ssl(server_url)) {
            // No additional configuration is necessary
            success = true;
            break;
        }

        if(!mqtt_load_ca_bundle(instance)) {
            // TODO: Preload CA bundle on startup
            break;
        }

        const char* ca_bundle = instance->ca_bundle;
        const MqttConfig* config = &instance->settings.config;

        if(!mqtt_tls_init(connection, server_url, ca_bundle, config)) {
            break;
        }

        success = true;
    } while(false);

    if(!success) {
        mqtt_connection_close(instance, false);
        mqtt_set_status(instance, MqttStatusError);
    }
}

static void mqtt_tls_handshake_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const void* event_data) {
    UNUSED(event_data);

    FURI_LOG_D(TAG, "TLS handshake done");
    // Free CA bundle data
    mqtt_tls_free_ca(connection);
    free(instance->ca_bundle);
    instance->ca_bundle = NULL;
}

static void mqtt_online_message_prepare(FuriString* message) {
    furi_string_set(message, "{");
    const Version* firmware_version = version_get();
    furi_string_cat_printf(
        message, "\"firmware_version\":\"%s\",", version_get_version(firmware_version));

    FuriString* version_string = furi_string_alloc();
    web_server_get_api_version(version_string);
    furi_string_cat_printf(
        message, "\"api_version\":\"%s\",", furi_string_get_cstr(version_string));
    furi_string_free(version_string);

    furi_string_cat_printf(message, STATUS_ONLINE);
    furi_string_cat(message, "}");
}

static void mqtt_online_message_send(Mqtt* instance) {
    FuriString* payload = furi_string_alloc();

    mqtt_online_message_prepare(payload);

    mqtt_publish_internal(
        instance,
        MqttScopeDevice,
        MqttQosAtLeastOnce,
        MQTT_TOPIC_PRESENCE,
        furi_string_get_cstr(payload),
        furi_string_size(payload),
        NULL,
        0);

    furi_string_free(payload);
}

static void mqtt_open_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    int* status_code_p) {
    UNUSED(connection);

    const int status_code = *status_code_p;

    if(status_code == 0) {
        FURI_LOG_I(TAG, "MQTT Connected");

        if(mqtt_saved_state_is_valid(&instance->saved_state)) {
            mqtt_set_status(instance, MqttStatusConnectedLinked);
        } else {
            mqtt_set_status(instance, MqttStatusConnectedNotLinked);
        }

        instance->reconnect_delay_ms = MQTT_RECONNECT_DELAY_MIN;

        MqttSubscriptionList_it_ct it;
        for(MqttSubscriptionList_it(it, instance->subscriptions); !MqttSubscriptionList_end_p(it);
            MqttSubscriptionList_next(it)) {
            mqtt_subscription_activate(instance, MqttSubscriptionList_cref(it));
        }

        mqtt_start_ping_timer(instance);
        mqtt_online_message_send(instance);

    } else {
        FURI_LOG_E(TAG, "MQTT Connect error, code 0x%02X", status_code);
    }
}

static void mqtt_close_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const void* event_data) {
    UNUSED(connection);
    UNUSED(event_data);

    FURI_LOG_W(TAG, "MQTT Connection closed");
    mqtt_set_status(instance, MqttStatusNotConnected);

    mqtt_stop_ping_timer(instance);

    instance->conn = NULL;

    if(instance->ca_bundle) {
        free(instance->ca_bundle);
        instance->ca_bundle = NULL;
    }

    if(instance->is_wifi_up) {
        if(instance->should_reconnect_now) {
            mqtt_connection_open(instance);
        } else {
            mqtt_start_reconnect_timer(instance);
        }
    }
}

static void mqtt_mqtt_cmd_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const struct mg_mqtt_message* message) {
    const uint8_t cmd = message->cmd;

    if(cmd == MQTT_CMD_SUBACK) {
        const size_t packet_len = message->dgram.len;
        const uint8_t sub_reason = message->dgram.buf[packet_len - 1];

        FURI_LOG_T(TAG, "MQTT SUBACK: 0x%02X", sub_reason);

        if(sub_reason >= MqttQosMax) {
            FURI_LOG_E(TAG, "Subscribe error 0x%02X", sub_reason);
            mqtt_connection_close(instance, false);
        }

    } else if(cmd == MQTT_CMD_PINGRESP) {
        FURI_LOG_D(TAG, "<- PONG");

    } else if(cmd == MQTT_CMD_PINGREQ) {
        FURI_LOG_D(TAG, "PING request received");
        mg_mqtt_pong(connection);

    } else {
        FURI_LOG_T(TAG, "MQTT CMD: %u", cmd);
    }
}

static struct mg_str mqtt_connection_trim_topic(struct mg_str topic_path) {
    struct mg_str captures[4]; // NOTE: Should be number of captures + 1
    struct mg_str pattern = mg_str("*/*/" MQTT_DIRECTION_DOWN "/" MQTT_API_VERSION "/#");

    if(mg_match(topic_path, pattern, captures)) {
        return captures[COUNT_OF(captures) - 2];

    } else {
        FURI_LOG_E(TAG, "Malformed topic path");
        return topic_path;
    }
}

static void mqtt_mqtt_msg_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const struct mg_mqtt_message* message) {
    UNUSED(connection);

    const struct mg_str topic = mqtt_connection_trim_topic(message->topic);

    MqttSubscriptionList_it_ct it;
    for(MqttSubscriptionList_it(it, instance->subscriptions); !MqttSubscriptionList_end_p(it);
        MqttSubscriptionList_next(it)) {
        const MqttSubscription* subscription = MqttSubscriptionList_cref(it);

        if(mg_strcmp(topic, mg_str(furi_string_get_cstr(subscription->topic))) == 0) {
            if(subscription->callback) {
                subscription->callback(TO_MQTT_MESSAGE(message), subscription->callback_context);
            }
        }
    }

    FURI_LOG_T(
        TAG,
        "MQTT MSG QOS%u %.*s :\r\n%.*s",
        message->qos,
        message->topic.len,
        message->topic.buf,
        message->data.len,
        message->data.buf);
}

static void mqtt_connection_mg_event_callback(
    struct mg_connection* connection,
    int event,
    void* event_data) {
    Mqtt* instance = connection->fn_data;
    furi_assert(instance);

    if(event == MG_EV_CONNECT) {
        mqtt_connect_mg_event_handler(instance, connection, event_data);
    } else if(event == MG_EV_TLS_HS) {
        mqtt_tls_handshake_mg_event_handler(instance, connection, event_data);
    } else if(event == MG_EV_MQTT_OPEN) {
        mqtt_open_mg_event_handler(instance, connection, event_data);
    } else if(event == MG_EV_CLOSE) {
        mqtt_close_mg_event_handler(instance, connection, event_data);
    } else if(event == MG_EV_MQTT_CMD) {
        mqtt_mqtt_cmd_mg_event_handler(instance, connection, event_data);
    } else if(event == MG_EV_MQTT_MSG) {
        mqtt_mqtt_msg_mg_event_handler(instance, connection, event_data);
    }
}

// Internal API

void mqtt_connection_open(Mqtt* instance) {
    mqtt_stop_reconnect_timer(instance);

    instance->should_reconnect_now = false;

    FuriString* username = furi_string_alloc_printf(
        "BusyBar device %s", furi_string_get_cstr(instance->device_serial));

    const MqttSavedState* saved_state = &instance->saved_state;

    FuriString* last_will_topic = furi_string_alloc();
    mqtt_make_topic_path(
        instance, MqttScopeDevice, MQTT_DIRECTION_UP, MQTT_TOPIC_PRESENCE, last_will_topic);

    const struct mg_mqtt_prop will_props[] = {{.id = MQTT_PROP_WILL_DELAY_INTERVAL, .iv = 0}};
    const struct mg_mqtt_opts opts = {
        .client_id = mg_str(saved_state->client_id),
        .user = mg_str(furi_string_get_cstr(username)),
        .pass = mg_str(saved_state->token),
        .clean = true,
        .keepalive = MQTT_PING_PERIOD / 1000,
        .version = MQTT_VERSION,

        .topic = mg_str(furi_string_get_cstr(last_will_topic)),
        .message = mg_str("{" STATUS_OFFLINE "}"),
        .qos = MqttQosAtLeastOnce,
        .will_props = (struct mg_mqtt_prop*)will_props,
        .num_will_props = COUNT_OF(will_props),
    };

    const char* server_url = mqtt_get_server_url(instance);
    FURI_LOG_D(TAG, "Connecting to %s ...", server_url);

    instance->conn = mg_mqtt_connect(
        &instance->mgr, server_url, &opts, mqtt_connection_mg_event_callback, instance);

    if(!instance->conn) {
        mqtt_set_status(instance, MqttStatusError);
    }

    furi_string_free(username);
    furi_string_free(last_will_topic);
}

void mqtt_connection_close(Mqtt* instance, bool reconnect_now) {
    if(instance->conn) {
        instance->conn->is_draining = true;
        instance->should_reconnect_now = reconnect_now;
    }
}
