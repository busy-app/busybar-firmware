
#include "mqtt_client_i.h"

#include <front_display/front_display.h>
#include <gui/gui.h>

#define TAG "MqttScreenStreaming"

#define MQTT_SCREEN_STREAMING_RESPONSE_QOS (0)
#define MQTT_SCREEN_STREAMING_FRAME_PERIOD (500)

static void mqtt_screen_streaming_timer_callback(void* data) {
    MqttClient* mqtt = data;
    furi_assert(mqtt->screen_stream_topic);
    furi_assert(mqtt->screen_stream_buf);

    Gui* gui = furi_record_open(RECORD_GUI);
    with_gui(gui, {
        const uint8_t* frame = gui_display_get_frame_buffer(gui, GuiDisplayIdFront);
        memcpy(mqtt->screen_stream_buf, frame, FRONT_DISPLAY_BUF_SIZE);
    });
    furi_record_close(RECORD_GUI);

    const struct mg_str message = {.buf = mqtt->screen_stream_buf, .len = FRONT_DISPLAY_BUF_SIZE};

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
        mqtt->screen_stream_buf = malloc(FRONT_DISPLAY_BUF_SIZE);
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
    if(!mqtt->screen_stream_topic) {
        mg_timer_free(&mqtt->mgr.timers, &mqtt->screen_stream_timer);
        free(mqtt->screen_stream_buf);
        furi_string_free(mqtt->screen_stream_topic);
        mqtt->screen_stream_topic = NULL;
    }
}

void mqtt_screen_streaming_on_message(
    MqttClient* mqtt,
    FuriString* topic_str,
    struct mg_mqtt_message* msg) {
    UNUSED(topic_str);

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
