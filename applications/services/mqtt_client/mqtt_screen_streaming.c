
#include "mqtt_client_i.h"

#include <front_display/front_display.h>
#include <gui/gui.h>

#define TAG "MqttScreenStreaming"

#define MQTT_SCREEN_STREAMING_REQUEST_QOS  (1)
#define MQTT_SCREEN_STREAMING_RESPONSE_QOS (0)

static char frame_buff[FRONT_DISPLAY_BUF_SIZE];

void mqtt_screen_streaming_subscribe(MqttClient* mqtt) {
    furi_assert(mqtt);
    furi_assert(mqtt->conn);

    FuriString* topic = furi_string_alloc_printf(
        "%s/%s/down/%s/stream-request",
        MQTT_API_ROOT_TOPIC,
        furi_string_get_cstr(mqtt->session_id),
        MQTT_API_VERSION);
    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(topic)), .qos = MQTT_SCREEN_STREAMING_REQUEST_QOS};
    mg_mqtt_sub(mqtt->conn, &opts);
    FURI_LOG_I(TAG, "Subscribing to topic: %s", furi_string_get_cstr(topic));

    furi_string_free(topic);
}

void mqtt_screen_streaming_timer_callback(void* context) {
    furi_assert(context);

    MqttClient* mqtt = context;

    Gui* gui = furi_record_open(RECORD_GUI);
    const uint8_t* frame = gui_display_get_frame_buffer(gui, GuiDisplayIdFront);
    memcpy(&frame_buff, frame, FRONT_DISPLAY_BUF_SIZE);
    furi_record_close(RECORD_GUI);

    const struct mg_str message = {.buf = frame_buff, .len = FRONT_DISPLAY_BUF_SIZE};

    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(mqtt->resp_topic)),
        .message = message,
        .qos = MQTT_SCREEN_STREAMING_RESPONSE_QOS,
        .retain = false,
        .props = NULL,
        .num_props = 0,

    };
    if(mqtt->conn) {
        mg_mqtt_pub(mqtt->conn, &opts);
    } else {
        FURI_LOG_E(TAG, "Connection lost");
    }
}

void mqtt_screen_streaming_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg) {
    furi_assert(mqtt);
    furi_assert(topic_str);
    furi_assert(msg);

    FuriString* cor_data = furi_string_alloc();
    struct mg_mqtt_prop prop;
    size_t prop_ofs = 0;
    do {
        memset(&prop, 0, sizeof(struct mg_mqtt_prop));
        prop_ofs = mg_mqtt_next_prop(msg, &prop, prop_ofs);
        if(prop_ofs <= 0) break;

        if(prop.id == MQTT_PROP_RESPONSE_TOPIC) {
            if(prop.val.len > 0) {
                furi_string_printf(mqtt->resp_topic, "%.*s", prop.val.len, prop.val.buf);
            }
            // } else if(prop.id == MQTT_PROP_CORRELATION_DATA) {
            //     if(prop.val.len > 0) {
            //         furi_string_printf(cor_data, "%.*s", prop.val.len, prop.val.buf);
            //     }
        }
    } while(prop_ofs > 0);

    if(furi_string_empty(mqtt->resp_topic)) {
        FURI_LOG_W(TAG, "Missing msg properties");
        furi_string_free(cor_data);
        furi_string_free(mqtt->resp_topic);
        return;
    }

    // struct mg_mqtt_prop props[] = {
    //     {
    //         .id = MQTT_PROP_CORRELATION_DATA,
    //         .val = mg_str(furi_string_get_cstr(cor_data)),
    //     },
    // };

    if(!furi_timer_is_running(mqtt->screen_streaming_timer)) {
        furi_timer_start(mqtt->screen_streaming_timer, 500);
    }
}
