#include "log_storage_local_i.h"

#define LOG_STORAGE_LOCAL_BUFFER_SIZE (16u * 1024u)
#define LOG_STORAGE_LOCAL_STREAM_SIZE (256u)

static void log_storage_local_on_log(const uint8_t* data, size_t size, void* context) {
    furi_assert(data);
    furi_assert(context);

    LogStorageLocal* instance = context;

    if(furi_stream_buffer_send(instance->rx_stream, data, size, 0) != size) {
        instance->did_overrun = true;
    }
}

static void log_storage_local_stream_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    LogStorageLocal* instance = context;

    furi_assert(instance->rx_stream == object);

    if(instance->did_overrun) {
        FURI_LOG_W(TAG, "Local log overrun occurred");
        instance->did_overrun = false;
    }

    log_storage_local_internal_flush(instance);
}

void log_storage_local_internal_flush(LogStorageLocal* instance) {
    furi_check(instance);

    uint8_t buffer[LOG_STORAGE_LOCAL_STREAM_SIZE];

    for(;;) {
        size_t rx_size =
            furi_stream_buffer_receive(instance->rx_stream, buffer, sizeof(buffer), 0);

        if(rx_size == 0) {
            break;
        }

        log_storage_base_internal_capture(&instance->base, buffer, rx_size);
    }
}

void log_storage_local_internal_init(LogStorageLocal* instance, FuriEventLoop* event_loop) {
    furi_check(instance);
    furi_check(event_loop);

    log_storage_base_internal_init(&instance->base, LOG_STORAGE_LOCAL_BUFFER_SIZE);
    instance->rx_stream = furi_stream_buffer_alloc(LOG_STORAGE_LOCAL_STREAM_SIZE, 1);
    instance->did_overrun = false;

    furi_event_loop_subscribe_stream_buffer(
        event_loop,
        instance->rx_stream,
        FuriEventLoopEventIn,
        log_storage_local_stream_callback,
        instance);

    furi_check(furi_log_add_handler((FuriLogHandler){
        .callback = log_storage_local_on_log,
        .context = instance,
    }));
}
