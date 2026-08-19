#include "log_storage_remote_i.h"

#include <furi_hal_serial_control.h>

#define LOG_STORAGE_REMOTE_BAUD_RATE   (230400u)
#define LOG_STORAGE_REMOTE_BUFFER_SIZE (16u * 1024u)
#define LOG_STORAGE_REMOTE_STREAM_SIZE (256u)

static void log_storage_remote_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent events,
    void* context) {
    furi_assert(handle);
    furi_assert(context);

    LogStorageRemote* instance = context;

    furi_assert(instance->serial == handle);

    if(events & FuriHalSerialRxEventData) {
        while(furi_hal_serial_rx_available(handle)) {
            uint8_t byte = furi_hal_serial_rx(handle);

            size_t rx_size = furi_stream_buffer_send(instance->rx_stream, &byte, sizeof(byte), 0);
            if(rx_size != sizeof(byte)) {
                instance->did_overrun = true;
            }
        }
    }
}

static void log_storage_remote_stream_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    LogStorageRemote* instance = context;

    furi_assert(instance->rx_stream == object);

    if(instance->did_overrun) {
        FURI_LOG_W(TAG, "Remote log overrun occurred");
        instance->did_overrun = false;
    }

    log_storage_remote_internal_flush(instance);
}

void log_storage_remote_internal_suspend(LogStorageRemote* instance) {
    furi_check(instance);

    furi_hal_serial_async_rx_stop(instance->serial);
    furi_hal_serial_set_rx_callback(instance->serial, NULL, NULL);
    furi_hal_serial_control_release(instance->serial);
    instance->serial = NULL;
}

void log_storage_remote_internal_resume(LogStorageRemote* instance) {
    furi_check(instance);

    instance->serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart2);
    furi_hal_serial_init(instance->serial, LOG_STORAGE_REMOTE_BAUD_RATE);
    furi_hal_serial_set_rx_callback(instance->serial, log_storage_remote_rx_callback, instance);
    furi_hal_serial_async_rx_start(instance->serial, false);
}

void log_storage_remote_internal_flush(LogStorageRemote* instance) {
    furi_check(instance);

    uint8_t buffer[LOG_STORAGE_REMOTE_STREAM_SIZE];
    for(;;) {
        size_t rx_size =
            furi_stream_buffer_receive(instance->rx_stream, buffer, sizeof(buffer), 0);

        if(rx_size == 0) {
            break;
        }

        log_storage_base_internal_capture(&instance->base, buffer, rx_size);
    }
}

void log_storage_remote_internal_init(LogStorageRemote* instance, FuriEventLoop* event_loop) {
    furi_check(instance);
    furi_check(event_loop);

    log_storage_base_internal_init(&instance->base, LOG_STORAGE_REMOTE_BUFFER_SIZE);
    instance->rx_stream = furi_stream_buffer_alloc(LOG_STORAGE_REMOTE_STREAM_SIZE, 1);
    instance->did_overrun = false;

    furi_event_loop_subscribe_stream_buffer(
        event_loop,
        instance->rx_stream,
        FuriEventLoopEventIn,
        log_storage_remote_stream_callback,
        instance);

    log_storage_remote_internal_resume(instance);
}
