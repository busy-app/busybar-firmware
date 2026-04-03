#include "mqtt_streaming_i.h"

#include <front_display/front_display.h>
#include <busy_timer/time_macros.h>
#include <cjson/cJSON.h>

#define TAG "MqttStreaming"

#define SUB_QOS (MqttQosAtLeastOnce)
#define PUB_QOS (MqttQosAtMostOnce)

#define SUB_TOPIC "stream-request"

#define API_QUEUE_SIZE  (4)
#define FRAME_PERIOD_MS (500)

#define EXPIRY_INTERVAL_DEFAULT_S (60)

static void mqtt_streaming_publish_callback(const SharedByteArray_t data, void* context);

static void mqtt_streaming_message_callback(const MqttMessage* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    MqttStreamingSrv* instance = context;

    size_t data_size;
    mqtt_message_get_data(message, &data_size);

    uint32_t expiry_interval = EXPIRY_INTERVAL_DEFAULT_S;
    mqtt_message_get_integer_property(message, MqttPropertyTypeExpiryInterval, &expiry_interval);

    FuriString* response_topic = furi_string_alloc();
    mqtt_message_get_string_property(message, MqttPropertyTypeResponseTopic, response_topic);

    size_t payload_size = 0;
    const void* payload = mqtt_message_get_data(message, &payload_size);
    void* own_payload = NULL;

    if(payload_size) {
        own_payload = malloc(payload_size);
        memcpy(own_payload, payload, payload_size);
    }

    const MqttStreamingApiMessage api_msg = {
        .type = data_size ? MqttStreamingApiMessageTypeStart : MqttStreamingApiMessageTypeStop,
        .expiry_interval = expiry_interval,
        .response_topic = response_topic,
        .payload = own_payload,
        .payload_size = payload_size,
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
                .response_topic = NULL,
            };

            furi_message_queue_put(instance->api_queue, &api_msg, FuriWaitForever);
        }
    }
}

static void stop_publisher(MqttStreamingSrv* instance) {
    if(instance->state_publisher_handle != STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID) {
        state_publisher_del_transport(instance->state_publisher, instance->state_publisher_handle);
        instance->state_publisher_handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;

        furi_string_free(instance->response_topic);
        instance->response_topic = NULL;
    }
}

static void mqtt_streaming_timeout_timer_callback(void* context) {
    furi_assert(context);
    MqttStreamingSrv* instance = context;

    FURI_LOG_I(TAG, "Stop (timeout)");

    stop_publisher(instance);
}

static RateLimiterLimit parse_rate_limit(const char* json, size_t length) {
    RateLimiterLimit result = RATE_LIMITER_UNLIMITED;
    cJSON* obj = cJSON_ParseWithLength(json, length);
    cJSON* limits_obj = cJSON_GetObjectItem(obj, "message_limits");
    cJSON* max_count_obj = cJSON_GetObjectItem(limits_obj, "max_count");
    cJSON* interval_obj = cJSON_GetObjectItem(limits_obj, "interval_s");

    if(cJSON_IsNumber(max_count_obj) && cJSON_IsNumber(interval_obj)) {
        double max_count = max_count_obj->valuedouble;
        double interval_s = interval_obj->valuedouble;
        if(max_count >= 0.0 && interval_s >= 0.0) {
            result.max_packet_count = (uint32_t)round(max_count);
            result.period_ms = (uint32_t)roundf((float)interval_s * 1000.0f);
            if(result.max_packet_count == 0) {
                result.period_ms = 0;
            }
        }
    }
    cJSON_Delete(obj);
    return result;
}

static void mqtt_streaming_api_queue_callback(FuriEventLoopObject* obj, void* context) {
    furi_assert(context);
    MqttStreamingSrv* instance = context;

    furi_assert(instance->api_queue == obj);

    MqttStreamingApiMessage api_msg;
    while(furi_message_queue_get(instance->api_queue, &api_msg, 0) == FuriStatusOk) {
        if(api_msg.type == MqttStreamingApiMessageTypeStart) {
            RateLimiterLimit rate_limit = parse_rate_limit(api_msg.payload, api_msg.payload_size);
            if(instance->state_publisher_handle == STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID) {
                FURI_LOG_I(TAG, "Start");
                instance->state_publisher_handle = state_publisher_add_transport(
                    instance->state_publisher,
                    StatePublisherTransportClassMQTT,
                    FRAME_PERIOD_MS,
                    rate_limit,
                    mqtt_streaming_publish_callback,
                    instance);
            } else {
                state_publisher_set_rate_limit(
                    instance->state_publisher, instance->state_publisher_handle, rate_limit);
                furi_string_free(instance->response_topic);
            }
            instance->response_topic = api_msg.response_topic;

            furi_event_loop_timer_start(instance->timeout_timer, S_TO_MS(api_msg.expiry_interval));

        } else if(api_msg.type == MqttStreamingApiMessageTypeStop) {
            FURI_LOG_I(TAG, "Stop");

            stop_publisher(instance);
            furi_event_loop_timer_stop(instance->timeout_timer);
            if(api_msg.response_topic) {
                furi_string_free(api_msg.response_topic);
            }
        } else {
            furi_crash("Invalid MqttStreamingApiMessageType value");
        }
        if(api_msg.payload) {
            free(api_msg.payload);
        }
    }
}

static void mqtt_streaming_publish_callback(const SharedByteArray_t data, void* context) {
    MqttStreamingSrv* instance = context;

    SharedByteArray_t my_data;
    SharedByteArray_init_set(my_data, data);

    furi_assert(instance->response_topic);
    furi_assert(furi_string_size(instance->response_topic));

    const ByteArray_t* array = SharedByteArray_cref(my_data);

    mqtt_publish(
        instance->mqtt,
        PUB_QOS,
        furi_string_get_cstr(instance->response_topic),
        ByteArray_cget(*array, 0),
        ByteArray_size(*array));
    SharedByteArray_clear(my_data);
}

static MqttStreamingSrv* mqtt_streaming_alloc(void) {
    MqttStreamingSrv* instance = malloc(sizeof(MqttStreamingSrv));

    instance->event_loop = furi_event_loop_alloc();
    instance->timeout_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        mqtt_streaming_timeout_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->api_queue =
        furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(MqttStreamingApiMessage));
    instance->mqtt = furi_record_open(RECORD_MQTT);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);
    instance->state_publisher_handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;
    instance->response_topic = NULL;

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
