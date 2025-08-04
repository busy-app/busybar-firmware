#include "mqtt_i.h"
#include <usb_network/usb_network.h>

#define TAG "MqttClient"

#define MQTT_SERVER_ADDR     "mqtt://10.0.4.21:1883"
#define MQTT_ROOT_TOPIC      "api/#"
#define MQTT_RECONNECT_DELAY (2000)
#define MQTT_QOS             (2)
#define HTTP_HOST            "http://127.0.0.1"

typedef struct {
    struct mg_mgr mgr;
    struct mg_timer reconnect_delay_timer;
    struct mg_connection* conn;
    bool conn_established;
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

    if(ev == MG_EV_MQTT_OPEN) {
        int* conn_code = (int*)ev_data;
        if(*conn_code == 0) {
            FURI_LOG_D(TAG, "MQTT Connected");
            const struct mg_mqtt_opts sub_opts = {
                .topic = mg_str(MQTT_ROOT_TOPIC), .qos = MQTT_QOS};
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
        FURI_LOG_T(TAG, "MQTT CMD: %u", msg->cmd);
    } else if(ev == MG_EV_MQTT_MSG) {
        struct mg_mqtt_message* msg = (struct mg_mqtt_message*)ev_data;
        if(msg->qos == MQTT_QOS) {
            mqtt_on_message(mqtt, &msg->topic, &msg->data);
        }
        FURI_LOG_T(
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
        .user = mg_str_s("user"),
        .pass = mg_str_s("pass"),
        .clean = true,
    };
    mqtt->conn = mg_mqtt_connect(&mqtt->mgr, MQTT_SERVER_ADDR, &opts, mqtt_event_handler, mqtt);
}

int32_t mqtt_client_start(void* p) {
    UNUSED(p);
    MqttClient* mqtt = malloc(sizeof(MqttClient));
    mqtt->conn = NULL;

    UsbNetwork* usb_network = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(usb_network);

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

    usb_network_thread_cleanup(usb_network);
    furi_record_close(RECORD_USB_NETWORK);

    free(mqtt);

    return 0;
}
