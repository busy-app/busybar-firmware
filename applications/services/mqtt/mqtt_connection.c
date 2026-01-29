#include "mqtt_i.h"

#include <storage/storage.h>
#include <busy_timer/time_macros.h>

#define TAG "MqttConnection"

#define MQTT_PING_PERIOD M_TO_MS(10)

#define CERT_FILE_CA_BUNDLE EXT_PATH("apps_assets/ca/cacert.pem")

static void mqtt_ping_timer_callback(void* data) {
    furi_assert(data);
    Mqtt* mqtt = data;

    if(mqtt->conn) {
        FURI_LOG_D(TAG, "-> PING");
        mg_mqtt_ping(mqtt->conn);
    }
}

static void mqtt_reconnect_callback(void* data) {
    furi_assert(data);
    Mqtt* instance = data;

    mqtt_connection_open(instance);
}

static bool mqtt_load_ca_bundle(Mqtt* mqtt) {
    furi_assert(mqtt->ca_bundle == NULL);
    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, CERT_FILE_CA_BUNDLE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "CA bundle file error: %s", storage_file_get_error_desc(file));
            break;
        }
        uint64_t file_size = storage_file_size(file);
        mqtt->ca_bundle = malloc(file_size);
        if(storage_file_read(file, mqtt->ca_bundle, file_size) != file_size) {
            FURI_LOG_E(TAG, "CA bundle file read error");
            break;
        }
        storage_file_close(file);

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

    if(!mqtt_load_ca_bundle(instance)) {
        connection->is_draining = 1;
        instance->status = MqttStatusError;
        return;
    }

    if(mqtt_is_tls_enabled(instance)) {
        const struct mg_str name = mg_url_host(mqtt_get_server_url(instance));
        const bool has_custom_certs = (instance->settings.profile_id == MqttProfileIdCustom);

        if(!mqtt_tls_init(connection, name, mg_str(instance->ca_bundle), has_custom_certs)) {
            connection->is_draining = 1;
            instance->status = MqttStatusError;
        }
    }
}

static void mqtt_tls_handshake_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const void* event_data) {
    UNUSED(event_data);

    FURI_LOG_D(TAG, "TLS handshake done!");
    // Free CA bundle data
    mqtt_tls_free_ca(connection);
    free(instance->ca_bundle);
    instance->ca_bundle = NULL;
}

static void mqtt_open_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    int* status_code_p) {
    UNUSED(connection);

    const int status_code = *status_code_p;

    if(status_code == 0) {
        FURI_LOG_I(TAG, "MQTT Connected");

        FuriString* topic_path = furi_string_alloc();

        MqttSubscriptionList_it_ct it;
        for(MqttSubscriptionList_it(it, instance->subscriptions); !MqttSubscriptionList_end_p(it);
            MqttSubscriptionList_next(it)) {
            const MqttSubscription* subscription = MqttSubscriptionList_cref(it);

            if(!mqtt_saved_state_is_valid(&instance->saved_state) &&
               subscription->scope == MqttScopeSession) {
                continue;
            }

            mqtt_make_topic_path(
                instance,
                subscription->scope,
                "down",
                furi_string_get_cstr(subscription->topic),
                topic_path);

            FURI_LOG_D(TAG, "Subscribing to %s", furi_string_get_cstr(topic_path));

            const struct mg_mqtt_opts sub_opts = {
                .topic = mg_str(furi_string_get_cstr(topic_path)),
                .qos = subscription->qos,
            };

            mg_mqtt_sub(instance->conn, &sub_opts);
        }

        furi_string_free(topic_path);

        // TODO: Refactor ping logic
        if(!instance->ping_enabled) {
            mg_timer_init(
                &instance->mgr.timers,
                &instance->ping_timer,
                MQTT_PING_PERIOD,
                MG_TIMER_REPEAT,
                mqtt_ping_timer_callback,
                instance);
            instance->ping_enabled = true;
        }

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

    if(instance->ping_enabled) {
        mg_timer_free(&instance->mgr.timers, &instance->ping_timer);
        instance->ping_enabled = false;
    }

    instance->conn = NULL;

    if(instance->ca_bundle) {
        free(instance->ca_bundle);
        instance->ca_bundle = NULL;
    }

    if(instance->is_wifi_up) {
        if(instance->fast_reconnect) {
            instance->fast_reconnect = false;
            mqtt_connection_open(instance);

        } else {
            mg_timer_init(
                &instance->mgr.timers,
                &instance->reconnect_delay_timer,
                instance->reconnect_delay,
                MG_TIMER_ONCE,
                mqtt_reconnect_callback,
                instance);
            instance->reconnect_delay *= 2;

            if(instance->reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
                instance->reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
            }
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

        FURI_LOG_D(TAG, "MQTT SUBACK: 0x%02X", sub_reason);

        if(sub_reason < MqttQosMax) {
            if(mqtt_saved_state_is_valid(&instance->saved_state)) {
                mqtt_set_status(instance, MqttStatusConnectedLinked);
            } else {
                mqtt_set_status(instance, MqttStatusConnectedNotLinked);
            }

            instance->reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

        } else {
            FURI_LOG_E(TAG, "Subscribe error 0x%02X", sub_reason);
            connection->is_draining = 1;
        }

    } else if(cmd == MQTT_CMD_PINGRESP) {
        FURI_LOG_D(TAG, "<- PONG");

    } else if(cmd == MQTT_CMD_PINGREQ) {
        FURI_LOG_D(TAG, "PING request received");
        mg_mqtt_pong(connection);

    } else {
        FURI_LOG_D(TAG, "MQTT CMD: %u", cmd);
    }
}

static void mqtt_mqtt_msg_mg_event_handler(
    Mqtt* instance,
    struct mg_connection* connection,
    const struct mg_mqtt_message* message) {
    UNUSED(connection);

    // TODO: Better way to match topics
    FuriString* topic_path =
        furi_string_alloc_printf("%.*s", message->topic.len, message->topic.buf);

    MqttSubscriptionList_it_ct it;
    for(MqttSubscriptionList_it(it, instance->subscriptions); !MqttSubscriptionList_end_p(it);
        MqttSubscriptionList_next(it)) {
        const MqttSubscription* subscription = MqttSubscriptionList_cref(it);

        if(furi_string_end_with(topic_path, subscription->topic)) {
            if(subscription->callback) {
                subscription->callback(TO_MQTT_MESSAGE(message), subscription->callback_context);
            }
        }
    }

    furi_string_free(topic_path);

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
    mg_timer_free(&instance->mgr.timers, &instance->reconnect_delay_timer);

    FuriString* username = furi_string_alloc_printf(
        "BusyBar device %s", furi_string_get_cstr(instance->device_serial));

    const MqttSavedState* saved_state = &instance->saved_state;

    const struct mg_mqtt_opts opts = {
        .client_id = mg_str(furi_string_get_cstr(saved_state->client_id)),
        .user = mg_str(furi_string_get_cstr(username)),
        .pass = mg_str(furi_string_get_cstr(saved_state->token)),
        .clean = true,
        .keepalive = 0,
        .version = 5,
    };

    const char* server_url = mqtt_get_server_url(instance);

    FURI_LOG_D(TAG, "Connecting to %s ...", server_url);

    instance->conn = mg_mqtt_connect(
        &instance->mgr, server_url, &opts, mqtt_connection_mg_event_callback, instance);

    if(!instance->conn) {
        instance->status = MqttStatusError;
    }

    furi_string_free(username);
}
