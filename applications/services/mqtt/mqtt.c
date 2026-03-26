#include "mqtt_i.h"

#include <furi_hal_random.h>
#include <furi_hal_version.h>

#include <network/network.h>
#include <storage/storage.h>

#include <toolbox/hex.h>

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

static void mqtt_init_device_uid(Mqtt* instance) {
    hex_bytes_to_string(
        furi_hal_version_uid(), furi_hal_version_uid_size(), instance->device_serial);
}

static void mqtt_load_settings(Mqtt* instance) {
    MqttSettings* settings = &instance->settings;
    mqtt_settings_load(settings);
}

static void mqtt_load_saved_state(Mqtt* instance) {
    MqttSavedState* saved_state = &instance->saved_state;
    mqtt_saved_state_load(saved_state);

    if(!mqtt_saved_state_is_valid(saved_state)) {
        FURI_LOG_W(TAG, "Saved state invalid, resetting");
        mqtt_reset_saved_state(instance);
    }
}

void mqtt_reset_saved_state(Mqtt* instance) {
    MqttSavedState* saved_state = &instance->saved_state;
    mqtt_saved_state_reset(saved_state);

    uint32_t random_id[2];
    furi_hal_random_fill_buf((uint8_t*)random_id, sizeof(random_id));

    snprintf(
        saved_state->client_id,
        sizeof(saved_state->client_id),
        "busybar-%08lx%08lx",
        random_id[0],
        random_id[1]);

    mqtt_saved_state_save(saved_state);
}

// Service thread

static Mqtt* mqtt_alloc(void) {
    Mqtt* instance = malloc(sizeof(Mqtt));

    instance->event_pubsub = furi_pubsub_alloc();
    instance->device_serial = furi_string_alloc();
    instance->reconnect_delay_ms = MQTT_RECONNECT_DELAY_MIN;
    instance->status = MqttStatusNotConnected;

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

    furi_record_create(RECORD_MQTT, instance);

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(wifi), mqtt_wifi_event_callback, instance);

    return instance;
}

int32_t mqtt_srv(void* arg) {
    UNUSED(arg);

    Mqtt* instance = mqtt_alloc();

    while(1) {
        mg_mgr_poll(&instance->mgr, MQTT_POLL_PERIOD_MS);
    }

    return 0;
}
