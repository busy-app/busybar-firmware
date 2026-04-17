#include "ble_streaming.h"
#include "state_publisher/state_publisher.h"

#define TAG "BleStream"

#define BLE_STREAM_LOCK_TIMEOUT_MS           (100)
#define BLE_STREAM_WAIT_TX_TIMEOUT_MS        (250)
#define BLE_STREAM_FRAME_PERIOD_MS           (1000)
#define BLE_STREAM_RATE_LIMITER_PERIOD_MS    (1000)
#define BLE_STREAM_RATE_LIMITER_MAX_PACK_CNT (1)

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
    BleStreamingEventFrameExit = (1 << 1)
} BleStreamingEvent;

struct BleStreaming {
    bool run;
    BleStreamingData send_buf;
    FuriMutex* lock;
    FuriSemaphore* wait_tx;
    StatePublisherTransportHandle handle;
    FuriEventLoop* event_loop;
    FuriThread* thread;
    SharedByteArray_t data;
    Ble* ble;
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

static void
    ble_streaming_send_data(BleStreaming* instance, const uint8_t* data, size_t data_size) {
    size_t index = 0;

    instance->send_buf.header.num = 0;
    instance->send_buf.header.count = ble_streaming_get_total_chunks_count(data_size);
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

        memset(instance->send_buf.data, 0, BLE_STREAMING_MAX_DATA_SIZE);
        data_size -= send_size;
        index += send_size;
        instance->send_buf.header.num += 1;
    }
}

static void ble_stream_state_publisher_callback(const SharedByteArray_t data, void* context) {
    BleStreaming* instance = context;

    if(furi_mutex_acquire(instance->lock, BLE_STREAM_LOCK_TIMEOUT_MS) == FuriStatusOk) {
        SharedByteArray_init_set(instance->data, data);
        furi_event_loop_set_custom_event(instance->event_loop, BleStreamingEventFramePending);
        furi_mutex_release(instance->lock);
    }
}

static inline void ble_stream_state_publisher_subscribe(BleStreaming* instance) {
    StatePublisher* state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);

    RateLimiterLimit limit = {
        .period_ms = BLE_STREAM_RATE_LIMITER_PERIOD_MS,
        .max_packet_count = BLE_STREAM_RATE_LIMITER_MAX_PACK_CNT,
    };

    instance->handle = state_publisher_add_transport(
        state_publisher,
        StatePublisherTransportClassBLE,
        BLE_STREAM_FRAME_PERIOD_MS,
        limit,
        ble_stream_state_publisher_callback,
        instance);

    furi_record_close(RECORD_STATE_PUBLISHER);
}

static inline void ble_stream_state_publisher_unsubscribe(BleStreaming* instance) {
    StatePublisher* state_publisher = furi_record_open(RECORD_STATE_PUBLISHER);
    state_publisher_del_transport(state_publisher, instance->handle);
    furi_record_close(RECORD_STATE_PUBLISHER);
    instance->handle = STATE_PUBLISHER_TRANSPORT_HANDLE_INVALID;
}

static inline void ble_stream_process_pending_frame(BleStreaming* instance) {
    if(furi_mutex_acquire(instance->lock, BLE_STREAM_LOCK_TIMEOUT_MS) == FuriStatusOk) {
        const ByteArray_t* array = SharedByteArray_cref(instance->data);
        const uint8_t* payload = ByteArray_cget(*array, 0);
        const size_t size = ByteArray_size(*array);

        ble_streaming_send_data(instance, payload, size);
        SharedByteArray_clear(instance->data);

        furi_mutex_release(instance->lock);
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
        FURI_LOG_D(TAG, "Stream start");
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
