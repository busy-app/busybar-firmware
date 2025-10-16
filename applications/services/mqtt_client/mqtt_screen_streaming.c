
#include "mqtt_client_i.h"

#include <front_display/front_display.h>
#include <gui/gui.h>

#define TAG "MqttScreenStreaming"

#define MQTT_SCREEN_STREAMING_REQUEST_QOS  (1)
#define MQTT_SCREEN_STREAMING_RESPONSE_QOS (0)
#define MQTT_SCREEN_STREAMING_FRAME_PERIOD (100)

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

static void mqtt_screen_streaming_timer_callback(void* data) {
    MqttClient* mqtt = data;
    furi_assert(mqtt);

    Gui* gui = furi_record_open(RECORD_GUI);
    with_gui(gui, {
        const uint8_t* frame = gui_display_get_frame_buffer(gui, GuiDisplayIdFront);
        memcpy(&frame_buff, frame, FRONT_DISPLAY_BUF_SIZE);
    });
    furi_record_close(RECORD_GUI);

    const struct mg_str message = {.buf = frame_buff, .len = FRONT_DISPLAY_BUF_SIZE};

    const struct mg_mqtt_opts opts = {
        .topic = mg_str(furi_string_get_cstr(mqtt->screen_stream_topic)),
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

static void mqtt_screen_streaming_start(MqttClient* mqtt, struct mg_str* topic_prop) {
    FURI_LOG_I(TAG, "Start");
    if(!mqtt->screen_stream_topic) {
        mqtt->screen_stream_topic = furi_string_alloc();
        mg_timer_init(
            &mqtt->mgr.timers,
            &mqtt->screen_stream_timer,
            MQTT_SCREEN_STREAMING_FRAME_PERIOD,
            MG_TIMER_REPEAT | MG_TIMER_RUN_NOW,
            mqtt_screen_streaming_timer_callback,
            mqtt);
    }
    furi_string_printf(mqtt->screen_stream_topic, "%.*s", topic_prop->len, topic_prop->buf);
}

static void mqtt_screen_streaming_stop(MqttClient* mqtt) {
    FURI_LOG_I(TAG, "End");
    mg_timer_free(&mqtt->mgr.timers, &mqtt->screen_stream_timer);
    furi_string_free(mqtt->screen_stream_topic);
    mqtt->screen_stream_topic = NULL;
}

void mqtt_screen_streaming_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg) {
    furi_assert(mqtt);
    furi_assert(topic_str);
    furi_assert(msg);

    if(msg->data.len == 0) {
        mqtt_screen_streaming_stop(mqtt);
    } else {
        // TODO: parse payload, timeout?
        struct mg_mqtt_prop prop;
        size_t prop_ofs = 0;
        do {
            memset(&prop, 0, sizeof(struct mg_mqtt_prop));
            prop_ofs = mg_mqtt_next_prop(msg, &prop, prop_ofs);
            if(prop_ofs <= 0) break;

            if((prop.id == MQTT_PROP_RESPONSE_TOPIC) && (prop.val.len > 0)) {
                mqtt_screen_streaming_start(mqtt, &prop.val);
            }
        } while(prop_ofs > 0);
    }
}
