#include "mqtt_client_i.h"

#include <furi_hal_random.h>
#include <furi_hal_version.h>

#include <network/network.h>
#include <storage/storage.h>

#include <toolbox/hex.h>

#define TAG "MqttClient"

#define CERT_FILE_CA_BUNDLE EXT_PATH("apps_assets/ca/cacert.pem")

#define MQTT_PING_PERIOD (10 * 60 * 1000)

typedef struct {
    const char* url;
    bool use_tls;
} MqttProfile;

static const MqttProfile mqtt_profiles[MqttProfileIdMax];

typedef void (*MqttApiMessageHandler)(MqttClient* instance, const MqttApiMessageData* data);

static const MqttApiMessageHandler mqtt_api_message_handlers[MqttApiMessageTypeMax];

static void mqtt_connect_callback(void* data);
static bool mqtt_client_load_ca_bundle(MqttClient* mqtt);
static void mqtt_reset_saved_state(MqttClient* instance);

static void mqtt_wifi_event_callback(const void* state, void* context) {
    MqttClient* instance = context;
    furi_assert(instance);

    const WifiInfo* info = state;

    const MqttApiMessage msg = {
        .type = MqttApiMessageTypeWifiState,
        .data.wifi_state =
            {
                .state = info->state,
            },
    };

    mg_wakeup(&instance->mgr, instance->wakeup_conn_id, &msg, sizeof(MqttApiMessage));
}

static void mqtt_ping_timer_callback(void* data) {
    furi_assert(data);
    MqttClient* mqtt = data;

    if(mqtt->conn) {
        FURI_LOG_D(TAG, "-> PING");
        mg_mqtt_ping(mqtt->conn);
    }
}

static void mqtt_set_status(MqttClient* instance, MqttClientStatus status) {
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

static const char* mqtt_get_server_url(const MqttClient* instance) {
    const MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    if(profile_id != MqttProfileIdCustom) {
        return mqtt_profiles[profile_id].url;
    } else {
        return furi_string_get_cstr(settings->custom_url);
    }
}

static bool mqtt_is_tls_enabled(const MqttClient* instance) {
    const MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    if(profile_id != MqttProfileIdCustom) {
        return mqtt_profiles[profile_id].use_tls;
    } else {
        return furi_string_start_with(settings->custom_url, MQTT_URL_TLS_PREFIX);
    }
}

static void mqtt_link_otp_subscription_callback(
    const char* topic,
    const void* data,
    size_t data_size,
    void* context) {
    UNUSED(topic);
    furi_assert(context);
    MqttClient* instance = context;

    const struct mg_str json_str = mg_str_n(data, data_size);

    char* pin = mg_json_get_str(json_str, "$.code");
    int32_t pin_expires_at = mg_json_get_long(json_str, "$.expires_at", -1);

    if(pin) {
        FURI_LOG_I(TAG, "Link PIN: %s", pin);
        MqttEvent pub_event = {
            .type = MqttEventTypeLinkPinReceived,
            .link_pin_received =
                {
                    .pin = pin,
                    .expires_at = pin_expires_at,
                },
        };

        furi_pubsub_publish(instance->event_pubsub, &pub_event);
        free(pin);
    }
}

static void mqtt_link_token_subscription_callback(
    const char* topic,
    const void* data,
    size_t data_size,
    void* context) {
    UNUSED(topic);
    furi_assert(context);
    MqttClient* instance = context;

    const struct mg_str json_str = mg_str_n(data, data_size);

    char* session_id = mg_json_get_str(json_str, "$.session_id");
    char* token = mg_json_get_str(json_str, "$.token");
    char* email = mg_json_get_str(json_str, "$.email");
    char* user_id = mg_json_get_str(json_str, "$.user_id");

    if(session_id && token && email && user_id) {
        FURI_LOG_I(TAG, "Link done!");

        MqttSavedState* saved_state = &instance->saved_state;

        furi_string_set(saved_state->session_id, session_id);
        furi_string_set(saved_state->user_id, user_id);
        furi_string_set(saved_state->email, email);
        furi_string_set(saved_state->token, token);

        mqtt_saved_state_save(saved_state);

        MqttEvent pub_event = {
            .type = MqttEventTypeLinkDone,
        };

        furi_pubsub_publish(instance->event_pubsub, &pub_event);

        // Close MQTT connection to reconnect with new token
        instance->conn->is_draining = 1;
        instance->fast_reconnect = true;
    }

    if(session_id) free(session_id);
    if(user_id) free(user_id);
    if(token) free(token);
    if(email) free(email);
}

// Mongoose event handlers

static void mqtt_connect_mg_event_handler(
    MqttClient* instance,
    struct mg_connection* connection,
    const void* event_data) {
    UNUSED(event_data);

    if(!mqtt_client_load_ca_bundle(instance)) {
        connection->is_draining = 1;
        instance->status = MqttClientStatusError;
        return;
    }

    if(mqtt_is_tls_enabled(instance)) {
        const struct mg_str name = mg_url_host(mqtt_get_server_url(instance));
        const bool has_custom_certs = (instance->settings.profile_id == MqttProfileIdCustom);

        if(!mqtt_tls_init(connection, name, mg_str(instance->ca_bundle), has_custom_certs)) {
            connection->is_draining = 1;
            instance->status = MqttClientStatusError;
        }
    }
}

static void mqtt_tls_handshake_mg_event_handler(
    MqttClient* instance,
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
    MqttClient* instance,
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
    MqttClient* instance,
    struct mg_connection* connection,
    const void* event_data) {
    UNUSED(connection);
    UNUSED(event_data);

    FURI_LOG_W(TAG, "MQTT Connection close");
    mqtt_set_status(instance, MqttClientStatusNotConnected);

    if(instance->ping_enabled) {
        mg_timer_free(&instance->mgr.timers, &instance->ping_timer);
        instance->ping_enabled = false;
    }

    instance->conn = NULL;

    if(instance->ca_bundle) {
        free(instance->ca_bundle);
        instance->ca_bundle = NULL;
    }

    mqtt_topics_on_close(instance);

    if(instance->is_wifi_up) {
        if(instance->fast_reconnect) {
            instance->fast_reconnect = false;
            mqtt_connect_callback(instance);

        } else {
            mg_timer_init(
                &instance->mgr.timers,
                &instance->reconnect_delay_timer,
                instance->reconnect_delay,
                MG_TIMER_ONCE,
                mqtt_connect_callback,
                instance);
            instance->reconnect_delay *= 2;

            if(instance->reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
                instance->reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
            }
        }
    }
}

static void mqtt_mqtt_cmd_mg_event_handler(
    MqttClient* instance,
    struct mg_connection* connection,
    const struct mg_mqtt_message* message) {
    const uint8_t cmd = message->cmd;

    if(cmd == MQTT_CMD_SUBACK) {
        const size_t packet_len = message->dgram.len;
        const uint8_t sub_reason = message->dgram.buf[packet_len - 1];

        FURI_LOG_D(TAG, "MQTT SUBACK: 0x%02X", sub_reason);

        if(sub_reason < MqttQosMax) {
            if(mqtt_saved_state_is_valid(&instance->saved_state)) {
                mqtt_set_status(instance, MqttClientStatusConnectedLinked);
            } else {
                mqtt_set_status(instance, MqttClientStatusConnectedNotLinked);
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
    MqttClient* instance,
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
        const char* topic_str = furi_string_get_cstr(subscription->topic);

        if(furi_string_end_with(topic_path, topic_str)) {
            if(subscription->callback) {
                subscription->callback(
                    topic_str,
                    message->data.buf,
                    message->data.len,
                    subscription->callback_context);
            }
        }
    }

    furi_string_free(topic_path);

    FURI_LOG_D(
        TAG,
        "MQTT MSG QOS%u %.*s : %.*s",
        message->qos,
        message->topic.len,
        message->topic.buf,
        message->data.len,
        message->data.buf);
}

static void mqtt_mg_connection_event_callback(
    struct mg_connection* connection,
    int event,
    void* event_data) {
    MqttClient* instance = connection->fn_data;
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

static void mqtt_connect_callback(void* data) {
    MqttClient* instance = data;
    furi_assert(instance);

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
        &instance->mgr, server_url, &opts, mqtt_mg_connection_event_callback, instance);

    if(!instance->conn) {
        instance->status = MqttClientStatusError;
    }

    furi_string_free(username);
}

static bool mqtt_client_load_ca_bundle(MqttClient* mqtt) {
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

static void mqtt_conn_wakeup_callback(struct mg_connection* conn, int ev, void* ev_data) {
    if(ev != MG_EV_WAKEUP) {
        return;
    }

    MqttClient* instance = conn->fn_data;
    furi_assert(instance);

    const struct mg_str* msg_data = ev_data;
    furi_assert(msg_data);
    furi_assert(msg_data->buf);
    furi_assert(msg_data->len == sizeof(MqttApiMessage));

    const MqttApiMessage* message = (MqttApiMessage*)(msg_data->buf);
    const MqttApiMessageType message_type = message->type;
    furi_assert(message_type < MqttApiMessageTypeMax);

    mqtt_api_message_handlers[message_type](instance, &message->data);

    if(message->lock) {
        api_lock_unlock(message->lock);
    }
}

// API message handlers

static void
    mqtt_get_status_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetStatus* get_status = &data->get_status;
    *(get_status->status) = instance->status;
}

static void mqtt_unlink_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    if(instance->conn) {
        instance->conn->is_draining = 1;
        instance->fast_reconnect = true;
    }

    mqtt_reset_saved_state(instance);
}

static void
    mqtt_request_pin_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    bool is_success = false;

    const MqttApiMessageRequestPin* request_pin = &data->request_pin;

    if(instance->status == MqttClientStatusConnectedNotLinked) {
        const char* empty = "{}";
        mqtt_publish_internal(
            instance, MqttScopeDevice, MqttQosExactlyOnce, "link/request", empty, strlen(empty));
        is_success = true;
    }

    *(request_pin->is_success) = is_success;
}

static void mqtt_get_session_info_api_message_handler(
    MqttClient* instance,
    const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetSessionInfo* get_session_info = &data->get_session_info;
    const MqttSavedState* saved_state = &instance->saved_state;

    if(get_session_info->session_id) {
        furi_string_set(get_session_info->session_id, saved_state->session_id);
    }
    if(get_session_info->user_id) {
        furi_string_set(get_session_info->user_id, saved_state->user_id);
    }
    if(get_session_info->email) {
        furi_string_set(get_session_info->email, saved_state->email);
    }
}

static void
    mqtt_get_profile_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageGetProfile* get_profile = &data->get_profile;

    MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    *(get_profile->profile_id) = profile_id;

    if(profile_id == MqttProfileIdCustom) {
        if(get_profile->custom_url) {
            furi_string_set(get_profile->custom_url, settings->custom_url);
        }
    }
}

static void
    mqtt_set_profile_api_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageSetProfile* set_profile = &data->set_profile;
    const MqttProfileId profile_id = set_profile->profile_id;

    MqttSettings* settings = &instance->settings;
    settings->profile_id = profile_id;

    if(profile_id == MqttProfileIdCustom) {
        // TODO: Better checks for custom url
        if(set_profile->custom_url) {
            furi_string_set(settings->custom_url, set_profile->custom_url);
        }
    }

    mqtt_settings_save(settings);

    if(instance->conn) {
        instance->conn->is_draining = 1;
    }
}

static void mqtt_publish_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessagePublish* publish = &data->publish;

    mqtt_publish_internal(
        instance,
        MqttScopeSession,
        publish->qos,
        publish->topic,
        publish->data,
        publish->data_size);
}

static void mqtt_subscribe_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageSubscribe* subscribe = &data->subscribe;
    *subscribe->subscription = mqtt_subscribe_internal(
        instance,
        MqttScopeSession,
        subscribe->qos,
        subscribe->topic,
        subscribe->callback,
        subscribe->callback_context);
}

static void
    mqtt_unsubscribe_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageUnsubscribe* unsubscribe = &data->unsubscribe;
    UNUSED(unsubscribe);
    // TODO: Implementation
}

static void mqtt_wifi_state_message_handler(MqttClient* instance, const MqttApiMessageData* data) {
    furi_assert(instance);
    furi_assert(data);

    const MqttApiMessageWifiState* wifi_state = &data->wifi_state;

    if(wifi_state->state == WifiStateConnected) {
        if((!instance->is_wifi_up) && (instance->conn == NULL)) {
            mqtt_connect_callback(instance);
        }

        instance->is_wifi_up = true;

    } else {
        instance->is_wifi_up = false;
    }
}

static void mqtt_init_device_uid(MqttClient* instance) {
    hex_bytes_to_string(
        furi_hal_version_uid(), furi_hal_version_uid_size(), instance->device_serial);
}

static void mqtt_load_settings(MqttClient* instance) {
    MqttSettings* settings = &instance->settings;

    mqtt_settings_init(settings);
    mqtt_settings_load(settings);
}

static void mqtt_reset_saved_state(MqttClient* instance) {
    MqttSavedState* saved_state = &instance->saved_state;
    mqtt_saved_state_reset(saved_state);

    uint32_t random_id[2];
    furi_hal_random_fill_buf((uint8_t*)random_id, sizeof(random_id));

    furi_string_printf(saved_state->client_id, "busybar-%08lx%08lx", random_id[0], random_id[1]);

    mqtt_saved_state_save(saved_state);
}

static void mqtt_load_saved_state(MqttClient* instance) {
    MqttSavedState* saved_state = &instance->saved_state;

    mqtt_saved_state_init(saved_state);
    mqtt_saved_state_load(saved_state);

    if(!mqtt_saved_state_is_valid(saved_state)) {
        FURI_LOG_W(TAG, "Saved state invalid, resetting");
        mqtt_reset_saved_state(instance);
    }
}

static void mqtt_api_init(MqttClient* instance) {
    // Create a dummy connection only for wakeup event
    const struct mg_connection* dummy_conn =
        mg_wrapfd(&instance->mgr, MG_INVALID_SOCKET, mqtt_conn_wakeup_callback, instance);
    instance->wakeup_conn_id = dummy_conn->id;
}

static void mqtt_device_topics_init(MqttClient* instance) {
    mqtt_subscribe_internal(
        instance,
        MqttScopeDevice,
        MqttQosExactlyOnce,
        "link/otp",
        mqtt_link_otp_subscription_callback,
        instance);

    mqtt_subscribe_internal(
        instance,
        MqttScopeDevice,
        MqttQosExactlyOnce,
        "link/token",
        mqtt_link_token_subscription_callback,
        instance);
}

// Constructor

static MqttClient* mqtt_client_alloc(void) {
    MqttClient* instance = malloc(sizeof(MqttClient));

    instance->status = MqttClientStatusNotConnected;
    instance->device_serial = furi_string_alloc();
    instance->event_pubsub = furi_pubsub_alloc();

    MqttSubscriptionList_init(instance->subscriptions);

    mqtt_init_device_uid(instance);

    mqtt_load_settings(instance);
    mqtt_load_saved_state(instance);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&instance->mgr);
    mg_wakeup_init(&instance->mgr);

    mqtt_api_init(instance);
    mqtt_device_topics_init(instance);

    instance->reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(wifi), mqtt_wifi_event_callback, instance);

    furi_record_create(RECORD_MQTT, instance);

    return instance;
}

// Service thread

int32_t mqtt_client_start(void* arg) {
    UNUSED(arg);

    MqttClient* instance = mqtt_client_alloc();

    while(1) {
        mg_mgr_poll(&instance->mgr, MQTT_POLL_PERIOD);
    }

    return 0;
}

// Subscribe & publish internal implementations

void mqtt_make_topic_path(
    MqttClient* instance,
    MqttScope scope,
    const char* dir,
    const char* topic,
    FuriString* out) {
    const char* root;
    const char* id;

    if(scope == MqttScopeDevice) {
        root = MQTT_DEVICE_ROOT_TOPIC;
        id = furi_string_get_cstr(instance->device_serial);

    } else if(scope == MqttScopeSession) {
        root = MQTT_SESSION_ROOT_TOPIC;
        id = furi_string_get_cstr(instance->saved_state.session_id);

    } else {
        furi_crash("Invalid MqttScope value");
    }

    furi_string_printf(out, "%s/%s/%s/%s/%s", root, id, dir, MQTT_API_VERSION, topic);
}

static MqttSubscription* mqtt_subscription_alloc(void) {
    MqttSubscription* subscription = malloc(sizeof(MqttSubscription));

    subscription->topic = furi_string_alloc();
    MqttSubscriptionList_init_field(subscription);

    return subscription;
}

// static void mqtt_subscription_free(MqttSubscription* subscription) {
//     furi_string_free(subscription->topic);
//     free(subscription);
// }

MqttSubscription* mqtt_subscribe_internal(
    MqttClient* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    MqttSubscriptionCallback callback,
    void* context) {
    MqttSubscription* subscription = mqtt_subscription_alloc();

    furi_string_set(subscription->topic, topic);
    subscription->scope = scope;
    subscription->qos = qos;
    subscription->callback = callback;
    subscription->callback_context = context;

    MqttSubscriptionList_push_back(instance->subscriptions, subscription);

    return subscription;
}

uint16_t mqtt_publish_internal(
    MqttClient* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size) {
    if(!instance->conn) {
        // TODO: What to do with messages published before the connection has been established?
        return 0;
    }

    FuriString* path = furi_string_alloc();
    mqtt_make_topic_path(instance, scope, "up", topic, path);

    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(path)),
        .message = mg_str_n(data, data_size),
        .qos = qos,
    };

    // TODO: Implement proper QoS handling
    const uint16_t retransmit_id = mg_mqtt_pub(instance->conn, &opts);

    furi_string_free(path);
    return retransmit_id;
}

// Static lookup tables

static const MqttApiMessageHandler mqtt_api_message_handlers[MqttApiMessageTypeMax] = {
    [MqttApiMessageTypeGetStatus] = mqtt_get_status_api_message_handler,
    [MqttApiMessageTypeUnlink] = mqtt_unlink_api_message_handler,
    [MqttApiMessageTypeRequestPin] = mqtt_request_pin_api_message_handler,
    [MqttApiMessageTypeGetSessionInfo] = mqtt_get_session_info_api_message_handler,
    [MqttApiMessageTypeGetProfile] = mqtt_get_profile_api_message_handler,
    [MqttApiMessageTypeSetProfile] = mqtt_set_profile_api_message_handler,
    [MqttApiMessageTypePublish] = mqtt_publish_message_handler,
    [MqttApiMessageTypeSubscribe] = mqtt_subscribe_message_handler,
    [MqttApiMessageTypeUnsubscribe] = mqtt_unsubscribe_message_handler,
    [MqttApiMessageTypeWifiState] = mqtt_wifi_state_message_handler,
};

static const MqttProfile mqtt_profiles[MqttProfileIdMax] = {
    [MqttProfileIdDevelopment] =
        {
            .url = MQTT_URL_TLS_PREFIX "mqtt.cloud.dev.busy.app:8883",
            .use_tls = true,
        },
    [MqttProfileIdProduction] =
        {
            .url = MQTT_URL_TLS_PREFIX "mqtt.cloud.dev.busy.app:8883",
            .use_tls = true,
        },
    [MqttProfileIdLocal] =
        {
            .url = MQTT_URL_PREFIX "10.0.4.21:1883",
            .use_tls = false,
        },
    [MqttProfileIdCustom] =
        {
            .url = NULL,
            .use_tls = false,
        },
};
