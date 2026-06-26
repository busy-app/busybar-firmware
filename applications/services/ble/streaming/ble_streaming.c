#include "ble_streaming.h"
#include "state_publisher/state_publisher.h"

#define TAG "BleStream"

#define BLE_STREAM_LOCK_TIMEOUT_MS            (100)
#define BLE_STREAM_WAIT_TX_TIMEOUT_MS         (250)
#define BLE_STREAM_FRAME_PERIOD_MS            (1000)
#define BLE_STREAM_RATE_LIMITER_PERIOD_STEP   (500)
#define BLE_STREAM_RATE_LIMITER_PERIOD_MS_MIN (1000)
#define BLE_STREAM_RATE_LIMITER_PERIOD_MS_MAX (5000)
#define BLE_STREAM_RATE_LIMITER_MAX_PACK_CNT  (1)

#define BLE_STREAM_SUCCESS_PACKET_CNT (10)

typedef struct FURI_PACKED {
    uint16_t num;
    uint16_t count;
    uint16_t size;
} BleStreamingDataHeader;

#define BLE_STREAMING_MAX_DATA_SIZE (MAX_TX_CHUNK_SIZE - sizeof(BleStreamingDataHeader))

typedef struct FURI_PACKED {
    BleStreamingDataHeader header;
    uint8_t data[BLE_STREAMING_MAX_DATA_SIZE];
} BleStreamingData;

typedef enum {
    BleStreamingEventFramePending = (1 << 0),
    BleStreamingEventIncreaseFramePeriod = (1 << 1),
    BleStreamingEventDecreaseFramePeriod = (1 << 2),
    BleStreamingEventFrameExit = (1 << 3)
} BleStreamingEvent;

struct BleStreaming {
    bool run;
    BleStreamingData send_buf;
    FuriMutex* lock;
    FuriSemaphore* wait_tx;
    StatePublisher* state_publisher;
    StatePublisherTransportHandle handle;
    FuriEventLoop* event_loop;
    FuriThread* thread;
    SharedByteArray_t data;
    Ble* ble;
    uint32_t period_ms;
    uint8_t packet_cnt;
};

static void ble_streaming_start(BleStreaming* instance);
static void ble_streaming_stop(BleStreaming* instance);

static void ble_uart_tx_done_callback(void* context) {
    BleStreaming* instance = context;
    furi_semaphore_release(instance->wait_tx);
}

static inline uint16_t ble_streaming_get_total_chunks_count(const size_t data_size) {
    uint16_t count = data_size / BLE_STREAMING_MAX_DATA_SIZE;
    if((data_size % BLE_STREAMING_MAX_DATA_SIZE) > 0) count += 1;
    return count;
}

static bool
    ble_streaming_send_data(BleStreaming* instance, const uint8_t* data, size_t data_size) {
    size_t index = 0;

    instance->send_buf.header.num = 0;
    instance->send_buf.header.count = ble_streaming_get_total_chunks_count(data_size);
    bool result = false;
    while(data_size && instance->run) {
        size_t send_size = data_size > BLE_STREAMING_MAX_DATA_SIZE ? BLE_STREAMING_MAX_DATA_SIZE :
                                                                     data_size;

        instance->send_buf.header.size = send_size;
        memcpy(instance->send_buf.data, &data[index], send_size);

        ble_uart_tx_data(
            instance->ble, BleUartChannelHM10, &instance->send_buf, sizeof(BleStreamingData));

        FuriStatus status =
            furi_semaphore_acquire(instance->wait_tx, BLE_STREAM_WAIT_TX_TIMEOUT_MS);
        if(status != FuriStatusOk) break;

        result = true;
        memset(instance->send_buf.data, 0, BLE_STREAMING_MAX_DATA_SIZE);
        data_size -= send_size;
        index += send_size;
        instance->send_buf.header.num += 1;
    }
    return result;
}

static void ble_stream_state_publisher_callback(const SharedByteArray_t data, void* context) {
    BleStreaming* instance = context;

    if(furi_mutex_acquire(instance->lock, BLE_STREAM_LOCK_TIMEOUT_MS) == FuriStatusOk) {
        SharedByteArray_init_set(instance->data, data);
        furi_event_loop_set_custom_event(instance->event_loop, BleStreamingEventFramePending);
        furi_mutex_release(instance->lock);
    } else {
        furi_event_loop_set_custom_event(
            instance->event_loop, BleStreamingEventIncreaseFramePeriod);
    }
}

static inline void ble_stream_state_publisher_subscribe(BleStreaming* instance) {
    instance->state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);

    RateLimiterLimit limit = {
        .period_ms = instance->period_ms,
        .max_packet_count = BLE_STREAM_RATE_LIMITER_MAX_PACK_CNT,
    };

    instance->handle = state_publisher_add_transport(
        instance->state_publisher,
        StatePublisherTransportClassBLE,
        BLE_STREAM_FRAME_PERIOD_MS,
        limit,
        ble_stream_state_publisher_callback,
        instance);
}

static inline void ble_stream_state_publisher_unsubscribe(BleStreaming* instance) {
    state_publisher_del_transport(instance->state_publisher, instance->handle);
    furi_record_close(RECORD_STATE_PUBLISHER);
    instance->state_publisher = NULL;
    instance->period_ms = BLE_STREAM_RATE_LIMITER_PERIOD_MS_MIN;
    instance->handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;
}

static void ble_stream_set_frame_period(BleStreaming* instance, int16_t step) {
    uint32_t new_period = instance->period_ms + step;

    if(instance->state_publisher == NULL) {
        FURI_LOG_W(TAG, "No state_publisher");
        return;
    }

    if(new_period >= BLE_STREAM_RATE_LIMITER_PERIOD_MS_MIN &&
       new_period <= BLE_STREAM_RATE_LIMITER_PERIOD_MS_MAX) {
        instance->period_ms = new_period;

        RateLimiterLimit limit = {
            .period_ms = instance->period_ms,
            .max_packet_count = BLE_STREAM_RATE_LIMITER_MAX_PACK_CNT,
        };
        state_publisher_set_rate_limit(instance->state_publisher, instance->handle, limit);

        instance->packet_cnt = 0;

        FURI_LOG_D(TAG, "New frame period: %ld", instance->period_ms);
    }
}

static void
    ble_stream_check_send_result_adjust_frame_period(BleStreaming* instance, bool send_done) {
    BleStreamingEvent event;
    bool trigger_frame_event = false;

    if(!send_done) {
        instance->packet_cnt = 0;
        event = BleStreamingEventIncreaseFramePeriod;
        trigger_frame_event = true;
    } else {
        event = BleStreamingEventDecreaseFramePeriod;
        instance->packet_cnt += 1;
        if(instance->packet_cnt == BLE_STREAM_SUCCESS_PACKET_CNT) {
            trigger_frame_event = (instance->period_ms > BLE_STREAM_RATE_LIMITER_PERIOD_MS_MIN);
            instance->packet_cnt = 0;
        }
    }

    if(trigger_frame_event) {
        furi_event_loop_set_custom_event(instance->event_loop, event);
    }
}

static inline void ble_stream_process_pending_frame(BleStreaming* instance) {
    if(furi_mutex_acquire(instance->lock, BLE_STREAM_LOCK_TIMEOUT_MS) == FuriStatusOk) {
        const ByteArray_t* array = SharedByteArray_cref(instance->data);
        const uint8_t* payload = ByteArray_cget(*array, 0);
        const size_t size = ByteArray_size(*array);

        bool result = ble_streaming_send_data(instance, payload, size);
        ble_stream_check_send_result_adjust_frame_period(instance, result);

        SharedByteArray_clear(instance->data);

        furi_mutex_release(instance->lock);
    } else {
        SharedByteArray_clear(instance->data);
    }
}

static inline void ble_stream_process_exit(BleStreaming* instance) {
    SharedByteArray_clear(instance->data);
    furi_event_loop_stop(instance->event_loop);
}

static void ble_streaming_event_loop_callback(uint32_t events, void* context) {
    BleStreaming* instance = context;

    if(events & BleStreamingEventFramePending) {
        ble_stream_process_pending_frame(instance);
    }

    if(events & BleStreamingEventIncreaseFramePeriod) {
        ble_stream_set_frame_period(instance, BLE_STREAM_RATE_LIMITER_PERIOD_STEP);
    }

    if(events & BleStreamingEventDecreaseFramePeriod) {
        ble_stream_set_frame_period(instance, -BLE_STREAM_RATE_LIMITER_PERIOD_STEP);
    }

    if(events & BleStreamingEventFrameExit) {
        ble_stream_process_exit(instance);
    }
}

static int32_t ble_streaming_thread(void* context) {
    BleStreaming* instance = context;

    ble_uart_set_tx_done_callback(
        instance->ble, BleUartChannelHM10, ble_uart_tx_done_callback, instance);
    ble_stream_state_publisher_subscribe(instance);

    instance->event_loop = furi_event_loop_alloc();
    furi_event_loop_set_custom_event_callback(
        instance->event_loop, ble_streaming_event_loop_callback, instance);

    ///TODO: Remove this when Ble streaming could be started from other side
    ///when connection parameters will be updated
    furi_delay_ms(5000);
    FURI_LOG_D(TAG, "Stream start");

    furi_event_loop_run(instance->event_loop);

    ble_stream_state_publisher_unsubscribe(instance);
    ble_uart_set_tx_done_callback(instance->ble, BleUartChannelHM10, NULL, NULL);

    furi_event_loop_free(instance->event_loop);
    return 0;
}

BleStreaming* ble_streaming_alloc(Ble* ble) {
    BleStreaming* instance = malloc(sizeof(BleStreaming));
    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->wait_tx = furi_semaphore_alloc(1, 0);
    instance->ble = ble;
    instance->handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;
    instance->run = false;
    instance->thread = furi_thread_alloc_ex(TAG, 1024, ble_streaming_thread, instance);
    instance->period_ms = BLE_STREAM_RATE_LIMITER_PERIOD_MS_MIN;
    return instance;
}

void ble_streaming_free(BleStreaming* instance) {
    furi_assert(instance);

    ble_streaming_stop(instance);
    furi_thread_free(instance->thread);
    furi_semaphore_free(instance->wait_tx);
    furi_mutex_free(instance->lock);
    free(instance);
}

static void ble_streaming_start(BleStreaming* instance) {
    furi_assert(instance);
    furi_mutex_acquire(instance->lock, FuriWaitForever);

    if(!instance->run) {
        instance->run = true;
        furi_thread_start(instance->thread);
    }
    furi_mutex_release(instance->lock);
}

static void ble_streaming_stop(BleStreaming* instance) {
    furi_assert(instance);
    furi_mutex_acquire(instance->lock, FuriWaitForever);

    if(instance->run) {
        instance->run = false;
        furi_event_loop_set_custom_event(instance->event_loop, BleStreamingEventFrameExit);
        furi_thread_join(instance->thread);
        FURI_LOG_D(TAG, "Stream stopped");
    }
    furi_mutex_release(instance->lock);
}

void ble_streaming_update(BleStreaming* instance, const BleServiceStatus status) {
    furi_assert(instance);
    furi_assert(status < BleServiceStatusCount);

    if(status == BleServiceStatusConnected)
        ble_streaming_start(instance);
    else
        ble_streaming_stop(instance);
}
