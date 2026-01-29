#include "mqtt_i.h"

#include <furi_hal_random.h>
#include <furi_hal_version.h>

#include <network/network.h>
#include <storage/storage.h>

#include <toolbox/hex.h>

typedef struct {
    const char* url;
    bool use_tls;
} MqttProfile;

static const MqttProfile mqtt_profiles[MqttProfileIdMax];

typedef enum {
    MqttPropertyValueTypeNumber,
    MqttPropertyValueTypeString,
    MqttPropertyValueTypeMax,
} MqttPropertyValueType;

typedef struct {
    uint8_t raw_id;
    MqttPropertyValueType value_type;
} MqttPropertyDesc;

static const MqttPropertyDesc mqtt_property_table[MqttPropertyTypeMax];

static void mqtt_wifi_event_callback(const void* state, void* context) {
    Mqtt* instance = context;
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

void mqtt_set_status(Mqtt* instance, MqttStatus status) {
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

bool mqtt_is_tls_enabled(const Mqtt* instance) {
    const MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    if(profile_id != MqttProfileIdCustom) {
        return mqtt_profiles[profile_id].use_tls;
    } else {
        return furi_string_start_with(settings->custom_url, MQTT_URL_TLS_PREFIX);
    }
}

const char* mqtt_get_server_url(const Mqtt* instance) {
    const MqttSettings* settings = &instance->settings;
    const MqttProfileId profile_id = settings->profile_id;

    if(profile_id != MqttProfileIdCustom) {
        return mqtt_profiles[profile_id].url;
    } else {
        return furi_string_get_cstr(settings->custom_url);
    }
}

static void mqtt_init_device_uid(Mqtt* instance) {
    hex_bytes_to_string(
        furi_hal_version_uid(), furi_hal_version_uid_size(), instance->device_serial);
}

static void mqtt_load_settings(Mqtt* instance) {
    MqttSettings* settings = &instance->settings;

    mqtt_settings_init(settings);
    mqtt_settings_load(settings);
}

void mqtt_reset_saved_state(Mqtt* instance) {
    MqttSavedState* saved_state = &instance->saved_state;
    mqtt_saved_state_reset(saved_state);

    uint32_t random_id[2];
    furi_hal_random_fill_buf((uint8_t*)random_id, sizeof(random_id));

    furi_string_printf(saved_state->client_id, "busybar-%08lx%08lx", random_id[0], random_id[1]);

    mqtt_saved_state_save(saved_state);
}

static void mqtt_load_saved_state(Mqtt* instance) {
    MqttSavedState* saved_state = &instance->saved_state;

    mqtt_saved_state_init(saved_state);
    mqtt_saved_state_load(saved_state);

    if(!mqtt_saved_state_is_valid(saved_state)) {
        FURI_LOG_W(TAG, "Saved state invalid, resetting");
        mqtt_reset_saved_state(instance);
    }
}

// Constructor

static Mqtt* mqtt_alloc(void) {
    Mqtt* instance = malloc(sizeof(Mqtt));

    instance->status = MqttStatusNotConnected;
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
    mqtt_account_init(instance);

    instance->reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(wifi), mqtt_wifi_event_callback, instance);

    furi_record_create(RECORD_MQTT, instance);

    return instance;
}

const void* mqtt_message_get_data(const MqttMessage* message, size_t* data_size) {
    furi_check(message);
    const struct mg_str data = TO_RAW_MESSAGE(message)->data;

    if(data_size) {
        *data_size = data.len;
    }

    return data.buf;
}

bool mqtt_message_get_string_property(
    const MqttMessage* message,
    MqttPropertyType property_type,
    FuriString* value) {
    furi_check(message);
    furi_check(property_type < MqttPropertyTypeMax);

    const MqttPropertyDesc* desc = &mqtt_property_table[property_type];
    furi_check(desc->value_type == MqttPropertyValueTypeString);

    bool is_found = false;

    for(size_t prop_offs = 0;;) {
        struct mg_mqtt_prop prop = {};
        const struct mg_mqtt_message* raw_message = TO_RAW_MESSAGE(message);
        // NOTE: mg_mqtt_next_prop() does NOT mutate data pointed to by *msg
        prop_offs = mg_mqtt_next_prop((struct mg_mqtt_message*)raw_message, &prop, prop_offs);

        if(prop_offs <= 0) {
            break;
        }

        if(prop.id == desc->raw_id) {
            if(value && prop.val.len) {
                furi_string_printf(value, "%.*s", prop.val.len, prop.val.buf);
                is_found = true;
                break;
            }
        }
    }

    return is_found;
}

void mqtt_make_topic_path(
    Mqtt* instance,
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
    Mqtt* instance,
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

void mqtt_unsubscribe_internal(Mqtt* instance, MqttSubscription* subscription) {
    UNUSED(instance);

    MqttSubscriptionList_unlink(subscription);
    mqtt_subscription_free(subscription);

    // TODO: reconnect
}

static void mqtt_property_to_raw(const MqttProperty* property, mg_mqtt_prop* raw_property) {
    const MqttPropertyDesc* desc = &mqtt_property_table[property->type];

    raw_property->id = desc->raw_id;

    if(desc->value_type == MqttPropertyValueTypeNumber) {
        raw_property->iv = property->value.number;
    } else if(desc->value_type == MqttPropertyValueTypeString) {
        raw_property->val = mg_str(property->value.string);
    } else {
        furi_crash("Invalid MqttPropertyValueType value");
    }
}

uint16_t mqtt_publish_internal(
    Mqtt* instance,
    MqttScope scope,
    MqttQos qos,
    const char* topic,
    const void* data,
    size_t data_size,
    const MqttProperty* props,
    uint32_t props_count) {
    if(!instance->conn) {
        // TODO: What to do with messages published before the connection has been established?
        return 0;
    }

    FuriString* path = furi_string_alloc();
    mqtt_make_topic_path(instance, scope, "up", topic, path);

    mg_mqtt_prop* raw_props = NULL;

    if(props && props_count) {
        raw_props = malloc(props_count * sizeof(mg_mqtt_prop));
        for(uint32_t i = 0; i < props_count; ++i) {
            mqtt_property_to_raw(&props[i], &raw_props[i]);
        }
    }

    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(path)),
        .message = mg_str_n(data, data_size),
        .qos = qos,
        .props = raw_props,
        .num_props = props_count,
    };

    // TODO: Implement proper QoS handling
    const uint16_t retransmit_id = mg_mqtt_pub(instance->conn, &opts);

    if(raw_props) {
        free(raw_props);
    }

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

static const MqttPropertyDesc mqtt_property_table[MqttPropertyTypeMax] = {
    [MqttPropertyTypeResponseTopic] =
        {
            .raw_id = MQTT_PROP_RESPONSE_TOPIC,
            .value_type = MqttPropertyValueTypeString,
        },
    [MqttPropertyTypeCorrelationData] =
        {
            .raw_id = MQTT_PROP_CORRELATION_DATA,
            .value_type = MqttPropertyValueTypeString,
        },
};

// Service thread

int32_t mqtt_srv(void* arg) {
    UNUSED(arg);

    Mqtt* instance = mqtt_alloc();

    while(1) {
        mg_mgr_poll(&instance->mgr, MQTT_POLL_PERIOD);
    }

    return 0;
}
