#include "intercom_i.h"

#define TAG "IntercomRxSrv"

#define INTERCOM_RX_THREAD_STACK_SIZE (2048)

typedef enum {
    IntercomRxThreadFlagFrameReceived = 1UL << 0,
} IntercomRxThreadFlag;

#ifdef SRV_INTERCOM_WATCHDOG

static FURI_ALWAYS_INLINE void intercom_rx_watchdog_begin(Intercom* instance) {
    intercom_watchdog_arm(instance->watchdog);
}

static FURI_ALWAYS_INLINE void intercom_rx_watchdog_end(Intercom* instance) {
    if(furi_hal_serial_rx_available(instance->serial)) {
        // Some more data is already in FIFO, re-arm watchdog
        intercom_watchdog_arm(instance->watchdog);
    } else {
        // No data in FIFO yet, disarm watchdog for now
        intercom_watchdog_disarm(instance->watchdog);
    }
}

#else

#define intercom_rx_watchdog_begin(instance) UNUSED(instance)
#define intercom_rx_watchdog_end(instance)   UNUSED(instance)

#endif // SRV_INTERCOM_WATCHDOG

// Called in ISR context
static void intercom_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    furi_assert(context);
    UNUSED(handle);

    const FuriThreadId rx_thread = context;

    if(event & FuriHalSerialRxEventData) {
        furi_thread_flags_set(rx_thread, IntercomRxThreadFlagFrameReceived);
    }
}

static FURI_ALWAYS_INLINE void intercom_rx_process_data(Intercom* instance) {
    INTERCOM_LOG_D("Frame received");

    const IntercomFrame* rx_frame = &instance->rx_frame;

    if(intercom_frame_is_valid(rx_frame)) {
        const IntercomChannelId channel_id = rx_frame->channel_id;
        const IntercomChannel* channel = &instance->handles[channel_id];

        intercom_rx_watchdog_begin(instance);
        intercom_channel_call_callback(channel, rx_frame);
        intercom_rx_watchdog_end(instance);

    } else {
        // TODO: Better error handling
        // intercom_error_handler(IntercomErrorFraming, instance);
    }
}

static void intercom_rx_init_serial(Intercom* instance) {
    furi_hal_serial_set_rx_callback(
        instance->serial, intercom_serial_rx_callback, furi_thread_get_current_id());
}

static void intercom_rx_wait_for_data(Intercom* instance) {
    furi_hal_serial_dma_rx_start(
        instance->serial, (void*)&instance->rx_frame, sizeof(IntercomFrame));

    const uint32_t flags = furi_thread_flags_wait(
        IntercomRxThreadFlagFrameReceived, FuriFlagWaitAny, FuriWaitForever);
    furi_check(flags == IntercomRxThreadFlagFrameReceived);
}

static int32_t intercom_rx_thread(void* arg) {
    furi_assert(arg);
    Intercom* instance = arg;

    intercom_rx_init_serial(instance);

    for(;;) {
        intercom_rx_wait_for_data(instance);
        intercom_rx_process_data(instance);
    }

    return 0;
}

// Private API

void intercom_start_rx_thread(Intercom* instance) {
    FuriThread* rx_thread = furi_thread_alloc_service(
        TAG, INTERCOM_RX_THREAD_STACK_SIZE, intercom_rx_thread, instance);
    furi_thread_start(rx_thread);
}
