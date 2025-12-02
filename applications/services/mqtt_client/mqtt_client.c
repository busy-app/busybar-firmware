#include "mqtt_client_i.h"
#include <network/network.h>
#include <storage/storage.h>
#include <json_helper.h>
#include <furi_hal_random.h>
#include <furi_hal_version.h>

#define TAG "MqttClient"

#define CERT_FILE_CA_BUNDLE EXT_PATH("apps_assets/ca/cacert.pem")

#define CONFIG_FILE  APP_DATA_PATH("config.json")
#define SESSION_FILE APP_DATA_PATH("session.json")

#define MQTT_PING_PERIOD (10 * 60 * 1000)

static struct {
    char* url;
    bool use_tls;
} server_profiles[MqttClientProfileMax] = {
    [MqttClientProfileDev] = {.url = "mqtts://mqtt.cloud.dev.busy.app:8883", .use_tls = true},
    [MqttClientProfileProd] =
        {.url = "mqtts://mqtt.cloud.dev.busy.app:8883", .use_tls = true}, // TODO: prod server
    [MqttClientProfileLocal] = {.url = "mqtt://10.0.4.21:1883", .use_tls = false},
};

static void mqtt_connect_callback(void* data);
static bool mqtt_client_load_ca_bundle(MqttClient* mqtt);

static void mqtt_wifi_event_callback(const void* state, void* context) {
    MqttClient* mqtt = context;
    furi_assert(mqtt);

    const WifiInfo* info = state;

    MqttClientMessage msg = {
        .type = MqttClientMessageWifiStateChange,
        .wifi_state = info->state,
        .lock = NULL,
    };

    mg_wakeup(&mqtt->mgr, mqtt->wakeup_conn_id, &msg, sizeof(MqttClientMessage));
}

static void mqtt_ping_timer_callback(void* data) {
    furi_assert(data);
    MqttClient* mqtt = data;

    if(mqtt->conn) {
        FURI_LOG_D(TAG, "-> PING");
        mg_mqtt_ping(mqtt->conn);
    }
}

static void mqtt_status_change_event(MqttClient* mqtt, MqttClientStatus status) {
    mqtt->status = status;
    MqttClientEvent pub_event = {.type = MqttClientEventStatusChange, .status = status};
    furi_pubsub_publish(mqtt->event_pubsub, &pub_event);
}

static void mqtt_device_subscribe(MqttClient* mqtt) {
    FuriString* topic = furi_string_alloc_printf(
        "%s/%s/down/%s/#",
        MQTT_DEVICE_ROOT_TOPIC,
        furi_string_get_cstr(mqtt->device_serial),
        MQTT_API_VERSION);
    const struct mg_mqtt_opts sub_opts = {
        .topic = mg_str(furi_string_get_cstr(topic)), .qos = MQTT_QOS};
    mg_mqtt_sub(mqtt->conn, &sub_opts);
    FURI_LOG_D(TAG, "Subscribing to %s", furi_string_get_cstr(topic));

    furi_string_free(topic);
}

static void mqtt_device_request_pin(MqttClient* mqtt) {
    FuriString* topic = furi_string_alloc_printf(
        "%s/%s/up/%s/link/request",
        MQTT_DEVICE_ROOT_TOPIC,
        furi_string_get_cstr(mqtt->device_serial),
        MQTT_API_VERSION);
    const struct mg_mqtt_opts pub_opts = {
        .topic = mg_str(furi_string_get_cstr(topic)),
        .message = mg_str("{}"),
        .qos = MQTT_QOS,
        .retain = false,
    };
    mg_mqtt_pub(mqtt->conn, &pub_opts);

    furi_string_free(topic);
}

static void
    mqtt_device_on_message(MqttClient* mqtt, FuriString* topic_str, struct mg_str* message) {
    if(furi_string_end_with(topic_str, "/down/v1/link/otp")) {
        char* pin = mg_json_get_str(*message, "$.code");
        int32_t pin_expires_at = mg_json_get_long(*message, "$.expires_at", -1);
        if(pin) {
            FURI_LOG_I(TAG, "Link PIN: %s", pin);
            MqttClientEvent pub_event = {
                .type = MqttClientEventLinkPin,
                .link = {.pin = pin, .expires_at = pin_expires_at}};
            furi_pubsub_publish(mqtt->event_pubsub, &pub_event);
            free(pin);
        }
    } else if(furi_string_end_with(topic_str, "/down/v1/link/token")) {
        char* session_id = mg_json_get_str(*message, "$.session_id");
        char* token = mg_json_get_str(*message, "$.token");
        char* email = mg_json_get_str(*message, "$.email");
        char* user_id = mg_json_get_str(*message, "$.user_id");
        if(session_id && token && email && user_id) {
            FURI_LOG_I(TAG, "Link done!");

            JsonConfig* cfg = json_config_alloc();
            JsonConfigStatus status = json_config_open(cfg, SESSION_FILE);
            furi_assert(status != JsonConfigStatusError);
            json_config_write_str(cfg, "session_id", session_id);
            json_config_write_str(cfg, "token", token);
            json_config_write_str(cfg, "email", email);
            json_config_write_str(cfg, "user_id", user_id);
            status = json_config_free(cfg);
            furi_assert(status != JsonConfigStatusError);

            furi_string_set(mqtt->session_id, session_id);
            furi_string_set(mqtt->link_token, token);
            mqtt->is_linked = true;

            MqttClientEvent pub_event = {.type = MqttClientEventLinkDone};
            furi_pubsub_publish(mqtt->event_pubsub, &pub_event);

            // Close MQTT connection to reconnect with new token
            mqtt->conn->is_draining = 1;
            mqtt->fast_reconnect = true;
        }
        if(session_id) free(session_id);
        if(token) free(token);
        if(email) free(email);
        if(user_id) free(user_id);
    }
}

static void mqtt_on_message(MqttClient* mqtt, struct mg_mqtt_message* msg) {
    furi_assert(mqtt);
    furi_assert(msg);

    FuriString* topic_str = furi_string_alloc_printf("%.*s", msg->topic.len, msg->topic.buf);

    if(furi_string_start_with(topic_str, MQTT_DEVICE_ROOT_TOPIC)) {
        mqtt_device_on_message(mqtt, topic_str, &msg->data);
    } else if(furi_string_start_with(topic_str, MQTT_API_ROOT_TOPIC)) {
        mqtt_topics_on_message(mqtt, topic_str, msg);
    }

    furi_string_free(topic_str);
}

static void mqtt_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    MqttClient* mqtt = conn->fn_data;
    furi_assert(mqtt);

    if(ev == MG_EV_CONNECT) {
        if(!mqtt_client_load_ca_bundle(mqtt)) {
            conn->is_draining = 1;
            mqtt->status = MqttClientStatusError;
            return;
        }
        if(mqtt->use_tls) {
            const struct mg_str name = mg_url_host(mqtt->server_addr);
            const MqttTlsCfg opts = {
                .name = name,
                .ca = mg_str(mqtt->ca_bundle),
            };
            if(!mqtt_tls_init(conn, &opts)) {
                conn->is_draining = 1;
                mqtt->status = MqttClientStatusError;
                return;
            }
        }
    } else if(ev == MG_EV_TLS_HS) {
        FURI_LOG_D(TAG, "TLS handshake done!");
        // Free CA bundle data
        mqtt_tls_free_ca(conn);
        free(mqtt->ca_bundle);
        mqtt->ca_bundle = NULL;
    } else if(ev == MG_EV_MQTT_OPEN) {
        int* conn_code = (int*)ev_data;
        if(*conn_code == 0) {
            FURI_LOG_I(TAG, "MQTT Connected");
            if(mqtt->is_linked) {
                mqtt_topics_subscribe(mqtt);
            } else {
                mqtt_device_subscribe(mqtt);
            }
            if(!mqtt->ping_enabled) {
                mg_timer_init(
                    &mqtt->mgr.timers,
                    &mqtt->ping_timer,
                    MQTT_PING_PERIOD,
                    MG_TIMER_REPEAT,
                    mqtt_ping_timer_callback,
                    mqtt);
                mqtt->ping_enabled = true;
            }
        } else {
            FURI_LOG_E(TAG, "MQTT Connect error, code 0x%02X", *conn_code);
        }
    } else if(ev == MG_EV_CLOSE) {
        FURI_LOG_W(TAG, "MQTT Connection close");
        mqtt_status_change_event(mqtt, MqttClientStatusNotConnected);

        if(mqtt->ping_enabled) {
            mg_timer_free(&mqtt->mgr.timers, &mqtt->ping_timer);
            mqtt->ping_enabled = false;
        }

        mqtt->conn = NULL;
        if(mqtt->ca_bundle) {
            free(mqtt->ca_bundle);
            mqtt->ca_bundle = NULL;
        }
        mqtt_topics_on_close(mqtt);
        if(mqtt->is_wifi_up) {
            if(mqtt->fast_reconnect) {
                mqtt->fast_reconnect = false;
                mqtt_connect_callback(mqtt);
            } else {
                mg_timer_init(
                    &mqtt->mgr.timers,
                    &mqtt->reconnect_delay_timer,
                    mqtt->reconnect_delay,
                    MG_TIMER_ONCE,
                    mqtt_connect_callback,
                    mqtt);
                mqtt->reconnect_delay *= 2;
                if(mqtt->reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
                    mqtt->reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
                }
            }
        }

    } else if(ev == MG_EV_MQTT_CMD) {
        struct mg_mqtt_message* msg = (struct mg_mqtt_message*)ev_data;
        if(msg->cmd == MQTT_CMD_SUBACK) {
            // Get SUBACK Reason Code from MQTT packet
            size_t packet_len = msg->dgram.len;
            uint8_t sub_reason = msg->dgram.buf[packet_len - 1];
            FURI_LOG_D(TAG, "MQTT SUBACK: 0x%02X", sub_reason);

            if(sub_reason == MQTT_QOS) {
                if(!mqtt->is_linked) {
                    mqtt_status_change_event(mqtt, MqttClientStatusConnectedNotLinked);
                } else {
                    mqtt_status_change_event(mqtt, MqttClientStatusConnectedLinked);
                }
                mqtt->reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
            } else {
                FURI_LOG_E(TAG, "Subscribe error 0x%02X", sub_reason);
                conn->is_draining = 1;
            }
        } else if(msg->cmd == MQTT_CMD_PINGRESP) {
            FURI_LOG_D(TAG, "<- PONG");
        } else if(msg->cmd == MQTT_CMD_PINGREQ) {
            FURI_LOG_D(TAG, "PING request received");
            mg_mqtt_pong(conn);
        } else {
            FURI_LOG_D(TAG, "MQTT CMD: %u", msg->cmd);
        }
    } else if(ev == MG_EV_MQTT_MSG) {
        struct mg_mqtt_message* msg = (struct mg_mqtt_message*)ev_data;

        mqtt_on_message(mqtt, msg);

        FURI_LOG_D(
            TAG,
            "MQTT MSG QOS%u %.*s : %.*s",
            msg->qos,
            msg->topic.len,
            msg->topic.buf,
            msg->data.len,
            msg->data.buf);
    }
}

static void mqtt_connect_callback(void* data) {
    MqttClient* mqtt = data;
    furi_assert(mqtt);

    mg_timer_free(&mqtt->mgr.timers, &mqtt->reconnect_delay_timer);

    FURI_LOG_D(TAG, "Connecting to %s ...", mqtt->server_addr);

    FuriString* username =
        furi_string_alloc_printf("BusyBar device %s", furi_string_get_cstr(mqtt->device_serial));

    const struct mg_mqtt_opts opts = {
        .client_id = mg_str(furi_string_get_cstr(mqtt->client_id)),
        .user = mg_str(furi_string_get_cstr(username)),
        .pass = mg_str(furi_string_get_cstr(mqtt->link_token)),
        .clean = true,
        .keepalive = 0,
        .version = 5,
    };
    mqtt->conn = mg_mqtt_connect(&mqtt->mgr, mqtt->server_addr, &opts, mqtt_event_handler, mqtt);

    furi_string_free(username);
}

static void mqtt_client_load_session(MqttClient* mqtt) {
    mqtt->is_linked = false;

    JsonConfig* cfg = json_config_alloc();
    JsonConfigStatus status = json_config_open(cfg, SESSION_FILE);
    furi_assert(status != JsonConfigStatusError);

    do {
        status = json_config_read_str(cfg, "client_id", mqtt->client_id, NULL);
        furi_assert(status != JsonConfigStatusError);
        if((status == JsonConfigStatusMissing) || (furi_string_empty(mqtt->client_id))) {
            uint32_t random_id[2];
            furi_hal_random_fill_buf((uint8_t*)random_id, sizeof(random_id));
            furi_string_printf(mqtt->client_id, "busybar-%08lx%08lx", random_id[0], random_id[1]);
            json_config_write_str(cfg, "client_id", furi_string_get_cstr(mqtt->client_id));
            break;
        }
        status = json_config_read_str(cfg, "session_id", mqtt->session_id, NULL);
        furi_assert(status != JsonConfigStatusError);
        if(status == JsonConfigStatusMissing) break;
        if(furi_string_empty(mqtt->session_id)) break;

        status = json_config_read_str(cfg, "token", mqtt->link_token, NULL);
        furi_assert(status != JsonConfigStatusError);
        if(status == JsonConfigStatusMissing) break;
        if(furi_string_empty(mqtt->link_token)) break;

        mqtt->is_linked = true;
    } while(0);

    if(!mqtt->is_linked) {
        furi_string_reset(mqtt->session_id);
        furi_string_reset(mqtt->link_token);
        json_config_delete(cfg, "session_id");
        json_config_delete(cfg, "token");
        FURI_LOG_W(TAG, "Session data reset");
    }

    status = json_config_free(cfg);
    furi_assert(status != JsonConfigStatusError);
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
    if(ev != MG_EV_WAKEUP) return;
    MqttClient* mqtt = conn->fn_data;
    furi_assert(mqtt);

    struct mg_str* msg_data = ev_data;

    furi_assert(msg_data->buf);
    furi_assert(msg_data->len == sizeof(MqttClientMessage));

    MqttClientMessage* msg = (MqttClientMessage*)(msg_data->buf);

    switch(msg->type) {
    case MqttClientMessageWifiStateChange:
        if(msg->wifi_state == WifiStateConnected) {
            if((!mqtt->is_wifi_up) && (mqtt->conn == NULL)) {
                mqtt_connect_callback(mqtt);
            }
            mqtt->is_wifi_up = true;
        } else {
            mqtt->is_wifi_up = false;
        }
        break;
    case MqttClientMessageGetStatus:
        *(msg->status) = mqtt->status;
        break;
    case MqttClientMessageRequestPin:
        if(mqtt->status == MqttClientStatusConnectedNotLinked) {
            mqtt_device_request_pin(mqtt);
            *(msg->bool_param) = true;
        } else {
            *(msg->bool_param) = false;
        }
        break;
    case MqttClientMessageUnlink:
        if(mqtt->conn) {
            mqtt->conn->is_draining = 1;
            mqtt->fast_reconnect = true;
        }
        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_common_remove(storage, SESSION_FILE);
        furi_record_close(RECORD_STORAGE);

        mqtt_client_load_session(mqtt);

        break;
    case MqttClientMessageGetSessionInfo:
        if(msg->session_info.id) {
            furi_string_set(msg->session_info.id, mqtt->session_id);
        }
        if(msg->session_info.email) {
            json_config_read_single_str(SESSION_FILE, "email", msg->session_info.email, "");
        }
        if(msg->session_info.user_id) {
            json_config_read_single_str(SESSION_FILE, "user_id", msg->session_info.user_id, "");
        }
        break;
    case MqttClientMessageGetProfile:
        furi_assert(msg->profile);
        *msg->profile = mqtt->profile_id;
        break;
    case MqttClientMessageSetProfile:
        furi_assert(msg->profile);
        furi_assert(*msg->profile < MqttClientProfileMax);
        json_config_write_single_int(CONFIG_FILE, "profile", *msg->profile);
        mqtt->profile_id = *msg->profile;
        mqtt->server_addr = server_profiles[mqtt->profile_id].url;
        mqtt->use_tls = server_profiles[mqtt->profile_id].use_tls;
        if(mqtt->conn) {
            mqtt->conn->is_draining = 1;
        }
        break;
    default:
        furi_crash();
        break;
    }

    if(msg->lock) {
        api_lock_unlock(msg->lock);
    }
}

int32_t mqtt_client_start(void* p) {
    UNUSED(p);
    MqttClient* mqtt = malloc(sizeof(MqttClient));
    mqtt->conn = NULL;
    mqtt->status = MqttClientStatusNotConnected;
    mqtt->ping_enabled = false;

    int default_profile = MqttClientProfileDev;
    furi_assert(
        json_config_read_single_int(CONFIG_FILE, "profile", &mqtt->profile_id, &default_profile) !=
        JsonConfigStatusError);
    if(mqtt->profile_id >= MqttClientProfileMax) {
        furi_assert(
            json_config_write_single_int(CONFIG_FILE, "profile", default_profile) !=
            JsonConfigStatusError);
        mqtt->profile_id = default_profile;
    }
    mqtt->server_addr = server_profiles[mqtt->profile_id].url;
    mqtt->use_tls = server_profiles[mqtt->profile_id].use_tls;

    mqtt->device_serial = furi_string_alloc();
    furi_hal_version_get_uid_str(mqtt->device_serial);

    mqtt->client_id = furi_string_alloc();
    mqtt->session_id = furi_string_alloc();
    mqtt->link_token = furi_string_alloc();

    // TODO: check certs + key on 917?

    mqtt_client_load_session(mqtt);

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&mqtt->mgr); // Initialise event manager

    mg_wakeup_init(&mqtt->mgr);
    // Create a dummy connection only for wakeup event
    struct mg_connection* dummy_conn =
        mg_wrapfd(&mqtt->mgr, MG_INVALID_SOCKET, mqtt_conn_wakeup_callback, mqtt);
    mqtt->wakeup_conn_id = dummy_conn->id;

    mqtt->event_pubsub = furi_pubsub_alloc();
    furi_record_create(RECORD_MQTT, mqtt);

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(wifi), mqtt_wifi_event_callback, mqtt);
    mqtt->is_wifi_up = false;

    mqtt->reconnect_delay = MQTT_RECONNECT_DELAY_MIN;

    if((mqtt->status != MqttClientStatusError) && (mqtt->is_wifi_up)) {
        mg_timer_init(
            &mqtt->mgr.timers,
            &mqtt->reconnect_delay_timer,
            mqtt->reconnect_delay,
            MG_TIMER_ONCE | MG_TIMER_RUN_NOW,
            mqtt_connect_callback,
            mqtt);
    }

    // Event loop
    while(1) {
        mg_mgr_poll(&mqtt->mgr, MQTT_POLL_PERIOD);
    }
    return 0;
}
