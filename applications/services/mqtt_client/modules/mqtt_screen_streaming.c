#include <gui/gui.h>
#include <mqtt_client/mqtt_client.h>
#include <front_display/front_display.h>

#include <time.h>

#define TAG "MqttScreenStreamSrv"

#define SUB_QOS (MqttQosAtLeastOnce)
#define PUB_QOS (MqttQosAtMostOnce)

#define SUB_TOPIC "stream-request"
#define PUB_TOPIC "displays/front"

#define API_QUEUE_SIZE  (4)
#define FRAME_PERIOD_MS (500)

typedef enum {
    MqttScreenStreamingApiMessageTypeStart,
    MqttScreenStreamingApiMessageTypeStop,
    MqttScreenStreamingApiMessageTypeMax,
} MqttScreenStreamingApiMessageType;

typedef struct {
    MqttScreenStreamingApiMessageType type;
    union {
        time_t timestamp;
    };
} MqttScreenStreamingApiMessage;

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriMessageQueue* api_queue;
    MqttClient* mqtt;
    Gui* gui;
    uint8_t* buffer;
} MqttScreenStreaming;

static void mqtt_screen_streaming_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttScreenStreaming* instance = context;

    size_t data_size;
    const char* data = mqtt_message_get_data(message, &data_size);
    UNUSED(data);
    // TODO: Parse timestamp & calculate timeout

    MqttScreenStreamingApiMessage api_msg;

    if(data_size) {
        api_msg.type = MqttScreenStreamingApiMessageTypeStart;
        api_msg.timestamp = 0;

    } else {
        api_msg.type = MqttScreenStreamingApiMessageTypeStop;
    }

    furi_message_queue_put(instance->api_queue, &api_msg, FuriWaitForever);
}

static void mqtt_screen_streaming_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttScreenStreaming* instance = context;

    const MqttEvent* event = message;
    if(event->type == MqttEventTypeStatusChanged) {
        if(event->status_changed.status == MqttClientStatusNotConnected) {
            FURI_LOG_E(TAG, "Connection lost");
            const MqttScreenStreamingApiMessage api_msg = {
                .type = MqttScreenStreamingApiMessageTypeStop,
            };

            furi_message_queue_put(instance->api_queue, &api_msg, FuriWaitForever);
        }
    }
}

static void mqtt_screen_streaming_timer_callback(void* context) {
    furi_assert(context);
    MqttScreenStreaming* instance = context;

    with_gui(instance->gui, {
        const uint8_t* frame = gui_display_get_frame_buffer(instance->gui, GuiDisplayIdFront);
        memcpy(instance->buffer, frame, FRONT_DISPLAY_BUF_SIZE);
    });

    mqtt_client_publish(
        instance->mqtt, PUB_QOS, PUB_TOPIC, instance->buffer, FRONT_DISPLAY_BUF_SIZE);
}

static void mqtt_screen_streaming_api_queue_callback(FuriEventLoopObject* obj, void* context) {
    furi_assert(context);
    MqttScreenStreaming* instance = context;

    furi_assert(instance->api_queue == obj);

    MqttScreenStreamingApiMessage msg;
    while(furi_message_queue_get(instance->api_queue, &msg, 0) == FuriStatusOk) {
        if(msg.type == MqttScreenStreamingApiMessageTypeStart) {
            if(!furi_event_loop_timer_is_running(instance->timer)) {
                FURI_LOG_I(TAG, "Start");
                furi_event_loop_timer_start(instance->timer, FRAME_PERIOD_MS);
            }

        } else if(msg.type == MqttScreenStreamingApiMessageTypeStop) {
            FURI_LOG_I(TAG, "Stop");
            furi_event_loop_timer_stop(instance->timer);

        } else {
            furi_crash("Invalid MqttScreenStreamingApiMessageType value");
        }
    }
}

static MqttScreenStreaming* mqtt_screen_streaming_alloc(void) {
    MqttScreenStreaming* instance = malloc(sizeof(MqttScreenStreaming));

    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        mqtt_screen_streaming_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->api_queue =
        furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(MqttScreenStreamingApiMessage));
    instance->mqtt = furi_record_open(RECORD_MQTT);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->buffer = malloc(FRONT_DISPLAY_BUF_SIZE);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        mqtt_screen_streaming_api_queue_callback,
        instance);

    furi_pubsub_subscribe(
        mqtt_client_get_pubsub(instance->mqtt), mqtt_screen_streaming_pubsub_callback, instance);

    mqtt_subscribe(
        instance->mqtt, SUB_QOS, SUB_TOPIC, mqtt_screen_streaming_message_callback, instance);

    return instance;
}

int32_t mqtt_screen_stream_srv(void* arg) {
    UNUSED(arg);

    MqttScreenStreaming* instance = mqtt_screen_streaming_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
