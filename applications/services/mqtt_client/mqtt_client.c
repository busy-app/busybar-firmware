#include "mqtt_i.h"
#include <network/network.h>
#include <storage/storage.h>

#define TAG "MqttClient"

#define CERT_FILE_CA_BUNDLE    APP_ASSETS_PATH("ca_bundle.crt")
#define CERT_FILE_INTERMEDIATE APP_ASSETS_PATH("signing-ca.crt")
#define CERT_FILE_DEVICE       APP_ASSETS_PATH("device.crt")
#define CERT_FILE_DEVICE_KEY   APP_ASSETS_PATH("device.key")

typedef struct {
    struct mg_mgr mgr;
    struct mg_timer reconnect_delay_timer;
    struct mg_connection* conn;
    bool conn_established;
    char* ca_bundle;
    char* device_cert;
    char* device_key;
} MqttClient;

typedef struct {
    MqttClient* mqtt;
    FuriString* response_topic;
    FuriString* http_request;
    char* request_data;
    size_t request_len;
} MqttHttpContext;

static void mqtt_http_handler(struct mg_connection* conn, int ev, void* ev_data) {
    MqttHttpContext* http_ctx = conn->fn_data;
    furi_assert(http_ctx);

    if(ev == MG_EV_CONNECT) {
        mg_printf(
            conn,
            "%s HTTP/1.1\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            furi_string_get_cstr(http_ctx->http_request),
            http_ctx->request_len);
        if(http_ctx->request_len > 0) {
            mg_send(conn, http_ctx->request_data, http_ctx->request_len);
        }
    } else if(ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* msg = (struct mg_http_message*)ev_data;
        FURI_LOG_T(TAG, "HTTP resp: %.*s", (int)msg->body.len, msg->body.buf);

        if((http_ctx->mqtt->conn_established) && (http_ctx->mqtt->conn)) {
            struct mg_mqtt_opts pub_opts = {
                .topic = mg_str(furi_string_get_cstr(http_ctx->response_topic)),
                .message = msg->body,
                .qos = MQTT_QOS,
                .retain = false,
            };
            mg_mqtt_pub(http_ctx->mqtt->conn, &pub_opts);
        } else {
            FURI_LOG_W(TAG, "MQTT died before response is sent");
        }
        conn->is_draining = 1;
    } else if(ev == MG_EV_CLOSE) {
        FURI_LOG_T(TAG, "HTTP close");

        furi_string_free(http_ctx->response_topic);
        furi_string_free(http_ctx->http_request);
        if(http_ctx->request_data) {
            free(http_ctx->request_data);
        }
        free(http_ctx);
        conn->fn_data = NULL;
    }
}

static void mqtt_on_message(MqttClient* mqtt, struct mg_str* topic, struct mg_str* message) {
    FuriString* http_req = furi_string_alloc();
    FuriString* resp_topic = furi_string_alloc();
    if(!mqtt_parse_topic(topic, http_req, resp_topic)) {
        FURI_LOG_W(TAG, "Unknown topic %.*s", topic->len, topic->buf);
        furi_string_free(http_req);
        furi_string_free(resp_topic);
        return;
    }

    MqttHttpContext* http_ctx = malloc(sizeof(MqttHttpContext));
    http_ctx->mqtt = mqtt;
    http_ctx->response_topic = resp_topic;
    http_ctx->http_request = http_req;
    http_ctx->request_len = message->len;
    if(http_ctx->request_len > 0) {
        http_ctx->request_data = malloc(message->len);
        memcpy(http_ctx->request_data, message->buf, message->len);
    }

    mg_http_connect(&mqtt->mgr, HTTP_HOST, mqtt_http_handler, http_ctx);
}

static void mqtt_reconnect_callback(void* data);

static void mqtt_event_handler(struct mg_connection* conn, int ev, void* ev_data) {
    MqttClient* mqtt = conn->fn_data;
    furi_assert(mqtt);

    if(ev == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(MQTT_SERVER_ADDR);
        const struct mg_tls_opts opts = {
            .name = name,
            .ca = mg_str(mqtt->ca_bundle),
            .cert = mg_str(mqtt->device_cert),
            .key = mg_str(mqtt->device_key),
            // .skip_verification = 1,
        };
        mg_tls_init(conn, &opts);
    } else if(ev == MG_EV_TLS_HS) {
        FURI_LOG_I(TAG, "TLS handshake done!");
    } else if(ev == MG_EV_MQTT_OPEN) {
        int* conn_code = (int*)ev_data;
        if(*conn_code == 0) {
            FURI_LOG_D(TAG, "MQTT Connected");
            const struct mg_mqtt_opts sub_opts = {
                .topic = mg_str(MQTT_SUBSCRIBE_TOPIC), .qos = MQTT_QOS};
            mg_mqtt_sub(conn, &sub_opts);
        } else {
            FURI_LOG_E(TAG, "MQTT Connect error, code 0x%02X", *conn_code);
        }
    } else if(ev == MG_EV_CLOSE) {
        FURI_LOG_E(TAG, "MQTT Connection close");
        mqtt->conn_established = false;
        mqtt->conn = NULL;
        mg_timer_init(
            &mqtt->mgr.timers,
            &mqtt->reconnect_delay_timer,
            MQTT_RECONNECT_DELAY,
            MG_TIMER_ONCE,
            mqtt_reconnect_callback,
            mqtt);
    } else if(ev == MG_EV_MQTT_CMD) {
        struct mg_mqtt_message* msg = (struct mg_mqtt_message*)ev_data;
        if(msg->cmd == MQTT_CMD_SUBACK) {
            mqtt->conn_established = true;
        }
        FURI_LOG_D(TAG, "MQTT CMD: %u", msg->cmd);
    } else if(ev == MG_EV_MQTT_MSG) {
        struct mg_mqtt_message* msg = (struct mg_mqtt_message*)ev_data;
        if(msg->qos == MQTT_QOS) {
            mqtt_on_message(mqtt, &msg->topic, &msg->data);
        }
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

static void mqtt_reconnect_callback(void* data) {
    MqttClient* mqtt = data;
    furi_assert(mqtt);

    mg_timer_free(&mqtt->mgr.timers, &mqtt->reconnect_delay_timer);

    FURI_LOG_D(TAG, "Connecting to %s ...", MQTT_SERVER_ADDR);
    const struct mg_mqtt_opts opts = {
        .user = mg_str_s("device_test"),
        .pass = mg_str_s(""),
        .clean = true,
    };
    mqtt->conn = mg_mqtt_connect(&mqtt->mgr, MQTT_SERVER_ADDR, &opts, mqtt_event_handler, mqtt);
}

static bool mqtt_client_load_certs(MqttClient* mqtt) {
    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        uint64_t file_size = 0;
        if(!storage_file_open(file, CERT_FILE_CA_BUNDLE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "CA bundle file error: %s", storage_file_get_error_desc(file));
            break;
        }
        file_size = storage_file_size(file);
        mqtt->ca_bundle = malloc(file_size);
        if(storage_file_read(file, mqtt->ca_bundle, file_size) != file_size) {
            FURI_LOG_E(TAG, "CA bundle file read error");
            break;
        }
        storage_file_close(file);

        FileInfo file_info;
        if(storage_common_stat(storage, CERT_FILE_INTERMEDIATE, &file_info) != FSE_OK) {
            FURI_LOG_E(TAG, "Intermediate cert file error");
            break;
        }
        uint64_t int_cert_size = file_info.size;

        if(!storage_file_open(file, CERT_FILE_DEVICE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Device cert file error: %s", storage_file_get_error_desc(file));
            break;
        }
        file_size = storage_file_size(file);
        mqtt->device_cert = malloc(int_cert_size + file_size);
        if(storage_file_read(file, mqtt->device_cert, file_size) != file_size) {
            FURI_LOG_E(TAG, "Device cert file read error");
            break;
        }
        storage_file_close(file);

        if(!storage_file_open(file, CERT_FILE_INTERMEDIATE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Intermediate cert file error: %s", storage_file_get_error_desc(file));
            break;
        }
        if(storage_file_read(file, &(mqtt->device_cert[file_size]), int_cert_size) !=
           int_cert_size) {
            FURI_LOG_E(TAG, "Intermediate cert file read error");
            break;
        }
        storage_file_close(file);

        if(!storage_file_open(file, CERT_FILE_DEVICE_KEY, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Device key file error: %s", storage_file_get_error_desc(file));
            break;
        }
        file_size = storage_file_size(file);
        mqtt->device_key = malloc(file_size);
        if(storage_file_read(file, mqtt->device_key, file_size) != file_size) {
            FURI_LOG_E(TAG, "Device key file read error");
            break;
        }
        storage_file_close(file);

        success = true;
    } while(0);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return success;
}

int32_t mqtt_client_start(void* p) {
    UNUSED(p);
    MqttClient* mqtt = malloc(sizeof(MqttClient));
    mqtt->conn = NULL;

    if(!mqtt_client_load_certs(mqtt)) {
        FURI_LOG_E(TAG, "Certificates load error");
        furi_thread_suspend(furi_thread_get_current_id());
    }

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&mqtt->mgr); // Initialise event manager

    mg_timer_init(
        &mqtt->mgr.timers,
        &mqtt->reconnect_delay_timer,
        MQTT_RECONNECT_DELAY,
        MG_TIMER_ONCE | MG_TIMER_RUN_NOW,
        mqtt_reconnect_callback,
        mqtt);

    // Event loop
    while(1) {
        mg_mgr_poll(&mqtt->mgr, 1000);
    }

    // Cleanup
    mg_mgr_free(&mqtt->mgr);

    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    free(mqtt);

    return 0;
}
