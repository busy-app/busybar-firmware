#include "mqtt_client_i.h"

#include <furi_hal_random.h>
#include <furi_hal_version.h>

#include <network/network.h>
#include <storage/storage.h>

#include <toolbox/hex.h>

#define TAG "MqttClient"

typedef struct {
    const char* url;
    bool use_tls;
} MqttProfile;

static const MqttProfile mqtt_profiles[MqttProfileIdMax];

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

    mg_wakeup(&instance->mgr, instance->api_connection_id, &msg, sizeof(MqttApiMessage));
}

void mqtt_set_status(MqttClient* instance, MqttClientStatus status) {
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

bool mqtt_is_tls_enabled(const MqttClient* instance) {
    const MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    if(profile_id != MqttProfileIdCustom) {
        return mqtt_profiles[profile_id].use_tls;
    } else {
        return furi_string_start_with(settings->custom_url, MQTT_URL_TLS_PREFIX);
    }
}

const char* mqtt_get_server_url(const MqttClient* instance) {
    const MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    if(profile_id != MqttProfileIdCustom) {
        return mqtt_profiles[profile_id].url;
    } else {
        return furi_string_get_cstr(settings->custom_url);
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

static void mqtt_init_device_uid(MqttClient* instance) {
    hex_bytes_to_string(
        furi_hal_version_uid(), furi_hal_version_uid_size(), instance->device_serial);
}

static void mqtt_load_settings(MqttClient* instance) {
    MqttSettings* settings = &instance->settings;

    mqtt_settings_init(settings);
    mqtt_settings_load(settings);
}

void mqtt_reset_saved_state(MqttClient* instance) {
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

static void mqtt_subscription_free(MqttSubscription* subscription) {
    furi_string_free(subscription->topic);
    free(subscription);
}

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

void mqtt_unsubscribe_internal(MqttClient* instance, MqttSubscription* subscription) {
    UNUSED(instance);

    MqttSubscriptionList_unlink(subscription);
    mqtt_subscription_free(subscription);

    // TODO: reconnect
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
