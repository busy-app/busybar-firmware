#include <gui/gui.h>
#include <mqtt_client/mqtt_client.h>
#include <front_display/front_display.h>

#define TAG "MqttScreenStreamingSrv"

#define THREAD_STACK_SIZE (1024)

#define SUB_QOS (MqttQosAtLeastOnce)
#define PUB_QOS (MqttQosAtMostOnce)

#define SUB_TOPIC "stream-request"
#define PUB_TOPIC "displays/front"

#define FRAME_PERIOD_MS (500)

typedef enum {
    MqttScreenStreamingThreadFlagStop = 1UL << 0,
} MqttScreenStreamingThreadFlag;

typedef struct {
    FuriThread* thread;
    uint8_t* buffer;
} MqttScreenStreaming;

static void mqtt_screen_streaming_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttScreenStreaming* instance = context;
    UNUSED(instance);

    size_t data_size;
    const char* data = mqtt_message_get_data(message, &data_size);
    UNUSED(data);
    // TODO: Parse timestamp & calculate timeout

    const FuriThreadState thread_state = furi_thread_get_state(instance->thread);

    if(data_size) {
        if(thread_state == FuriThreadStateStopping) {
            furi_thread_join(instance->thread);
        }
        if(thread_state == FuriThreadStateStopped) {
            furi_thread_start(instance->thread);
        }

    } else {
        if(thread_state == FuriThreadStateRunning || thread_state == FuriThreadStateStarting) {
            furi_thread_flags_set(instance->thread, MqttScreenStreamingThreadFlagStop);
        }

        furi_thread_join(instance->thread);
    }
}

static void mqtt_screen_streaming_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttScreenStreaming* instance = context;

    const MqttEvent* event = message;
    if(event->type == MqttEventTypeStatusChanged) {
        if(event->status_changed.status == MqttClientStatusNotConnected) {
            FURI_LOG_E(TAG, "Connection lost");
            furi_thread_flags_set(instance->thread, MqttScreenStreamingThreadFlagStop);
            furi_thread_join(instance->thread);
        }
    }
}

static int32_t mqtt_screen_streaming_thread_callback(void* arg) {
    furi_assert(arg);
    MqttScreenStreaming* instance = arg;

    furi_check(instance->buffer == NULL);
    instance->buffer = malloc(FRONT_DISPLAY_BUF_SIZE);

    Gui* gui = furi_record_open(RECORD_GUI);
    MqttClient* mqtt = furi_record_open(RECORD_MQTT);

    FURI_LOG_I(TAG, "Start");

    for(;;) {
        const uint32_t flags = furi_thread_flags_wait(
            MqttScreenStreamingThreadFlagStop, FuriFlagWaitAny, FRAME_PERIOD_MS);

        if(flags & FuriFlagError) {
            furi_check(flags == FuriFlagErrorTimeout);
        } else if(flags & MqttScreenStreamingThreadFlagStop) {
            break;
        }

        with_gui(gui, {
            const uint8_t* frame = gui_display_get_frame_buffer(gui, GuiDisplayIdFront);
            memcpy(instance->buffer, frame, FRONT_DISPLAY_BUF_SIZE);
        });

        mqtt_client_publish(mqtt, PUB_QOS, PUB_TOPIC, instance->buffer, FRONT_DISPLAY_BUF_SIZE);
    }

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_MQTT);

    free(instance->buffer);
    instance->buffer = NULL;

    FURI_LOG_I(TAG, "Stop");

    return 0;
}

static MqttScreenStreaming* mqtt_screen_streaming_alloc(void) {
    MqttScreenStreaming* instance = malloc(sizeof(MqttScreenStreaming));
    instance->thread = furi_thread_alloc_ex(
        TAG, THREAD_STACK_SIZE, mqtt_screen_streaming_thread_callback, instance);
    return instance;
}

void mqtt_screen_streaming_on_system_start(void) {
    MqttScreenStreaming* instance = mqtt_screen_streaming_alloc();

    MqttClient* mqtt = furi_record_open(RECORD_MQTT);

    furi_pubsub_subscribe(
        mqtt_client_get_pubsub(mqtt), mqtt_screen_streaming_pubsub_callback, instance);

    mqtt_subscribe(mqtt, SUB_QOS, SUB_TOPIC, mqtt_screen_streaming_message_callback, instance);
}
