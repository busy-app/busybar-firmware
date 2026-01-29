#include "mqtt_streaming_i.h"

#include <front_display/front_display.h>
#include <busy_timer/time_macros.h>

#define TAG "MqttStreaming"

#define SUB_QOS (MqttQosAtLeastOnce)
#define PUB_QOS (MqttQosAtMostOnce)

#define SUB_TOPIC "stream-request"
#define PUB_TOPIC "displays/front"

#define API_QUEUE_SIZE  (4)
#define FRAME_PERIOD_MS (500)

#define STREAM_TIMEOUT_MS M_TO_MS(1)

static void mqtt_streaming_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttStreamingSrv* instance = context;

    size_t data_size;
    mqtt_message_get_data(message, &data_size);

    const MqttStreamingApiMessage api_msg = {
        .type = data_size ? MqttStreamingApiMessageTypeStart : MqttStreamingApiMessageTypeStop,
    };

    furi_message_queue_put(instance->api_queue, &api_msg, FuriWaitForever);
}

static void mqtt_streaming_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttStreamingSrv* instance = context;

    const MqttEvent* event = message;
    if(event->type == MqttEventTypeStatusChanged) {
        if(event->status_changed.status == MqttStatusNotConnected) {
            FURI_LOG_E(TAG, "Connection lost");
            const MqttStreamingApiMessage api_msg = {
                .type = MqttStreamingApiMessageTypeStop,
            };

            furi_message_queue_put(instance->api_queue, &api_msg, FuriWaitForever);
        }
    }
}

static void mqtt_streaming_frame_timer_callback(void* context) {
    furi_assert(context);
    MqttStreamingSrv* instance = context;

    with_gui(instance->gui, {
        const uint8_t* frame = gui_display_get_frame_buffer(instance->gui, GuiDisplayIdFront);
        memcpy(instance->frame_buf, frame, FRONT_DISPLAY_BUF_SIZE);
    });

    mqtt_publish(instance->mqtt, PUB_QOS, PUB_TOPIC, instance->frame_buf, FRONT_DISPLAY_BUF_SIZE);
}

static void mqtt_streaming_timeout_timer_callback(void* context) {
    furi_assert(context);
    MqttStreamingSrv* instance = context;

    FURI_LOG_I(TAG, "Stop (timeout)");

    furi_event_loop_timer_stop(instance->frame_timer);
}

static void mqtt_streaming_api_queue_callback(FuriEventLoopObject* obj, void* context) {
    furi_assert(context);
    MqttStreamingSrv* instance = context;

    furi_assert(instance->api_queue == obj);

    MqttStreamingApiMessage api_msg;
    while(furi_message_queue_get(instance->api_queue, &api_msg, 0) == FuriStatusOk) {
        if(api_msg.type == MqttStreamingApiMessageTypeStart) {
            if(!furi_event_loop_timer_is_running(instance->frame_timer)) {
                FURI_LOG_I(TAG, "Start");

                furi_event_loop_timer_start(instance->frame_timer, FRAME_PERIOD_MS);
                furi_event_loop_pend_callback(
                    instance->event_loop, mqtt_streaming_frame_timer_callback, instance);
            }

            furi_event_loop_timer_start(instance->timeout_timer, STREAM_TIMEOUT_MS);

        } else if(api_msg.type == MqttStreamingApiMessageTypeStop) {
            FURI_LOG_I(TAG, "Stop");

            furi_event_loop_timer_stop(instance->frame_timer);
            furi_event_loop_timer_stop(instance->timeout_timer);

        } else {
            furi_crash("Invalid MqttStreamingApiMessageType value");
        }
    }
}

static MqttStreamingSrv* mqtt_streaming_alloc(void) {
    MqttStreamingSrv* instance = malloc(sizeof(MqttStreamingSrv));

    instance->event_loop = furi_event_loop_alloc();
    instance->frame_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        mqtt_streaming_frame_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->timeout_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        mqtt_streaming_timeout_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->api_queue =
        furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(MqttStreamingApiMessage));
    instance->mqtt = furi_record_open(RECORD_MQTT);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->frame_buf = malloc(FRONT_DISPLAY_BUF_SIZE);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        mqtt_streaming_api_queue_callback,
        instance);

    furi_pubsub_subscribe(
        mqtt_get_pubsub(instance->mqtt), mqtt_streaming_pubsub_callback, instance);

    mqtt_subscribe(instance->mqtt, SUB_QOS, SUB_TOPIC, mqtt_streaming_message_callback, instance);

    return instance;
}

int32_t mqtt_streaming_srv(void* arg) {
    UNUSED(arg);

    MqttStreamingSrv* instance = mqtt_streaming_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
