#include "intercom_i.h"

#define TAG "IntercomRx"

#define INTERCOM_RX_THREAD_NAME       TAG "Srv"
#define INTERCOM_RX_THREAD_STACK_SIZE (2048)

typedef enum {
    IntercomRxThreadFlagFrameReceived = 1UL << 0,
    IntercomRxThreadFlagErrorOccurred = 1UL << 1,
} IntercomRxThreadFlag;

#define INTERCOM_RX_THREAD_FLAGS_ALL \
    (IntercomRxThreadFlagFrameReceived | IntercomRxThreadFlagErrorOccurred)

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

static void intercom_rx_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    const FuriThreadId rx_thread = context;
    const IntercomStatus status = *(IntercomStatus*)item;

    if(status != IntercomStatusOk) {
        furi_thread_flags_set(rx_thread, IntercomRxThreadFlagErrorOccurred);
    }
}

static FURI_ALWAYS_INLINE void
    intercom_rx_process_frame(Intercom* instance, const IntercomFrame* frame) {
    const IntercomChannelId channel_id = frame->channel_id;

    if(channel_id < IntercomChannelIdMax) {
        const IntercomChannel* channel = &instance->channels[channel_id];
        intercom_channel_call_callback(channel, frame);
    } else {
        intercom_meta_process_frame(instance, frame);
    }
}

static FURI_ALWAYS_INLINE void intercom_rx_process_data(Intercom* instance) {
    INTERCOM_LOG_D("Frame received");

    const IntercomFrame* rx_frame = &instance->rx_frame;

    if(intercom_frame_is_valid(rx_frame)) {
        intercom_rx_process_frame(instance, rx_frame);

    } else {
        intercom_set_status(instance, IntercomStatusErrorFraming);
        intercom_dump_frame(rx_frame);
        furi_thread_suspend(furi_thread_get_current_id());
    }
}

static void intercom_rx_init_serial(Intercom* instance) {
    furi_hal_serial_set_rx_callback(
        instance->serial, intercom_serial_rx_callback, furi_thread_get_current_id());
}

static void intercom_rx_wait_for_data(Intercom* instance) {
    furi_hal_serial_dma_rx_start(
        instance->serial, (void*)&instance->rx_frame, sizeof(IntercomFrame));

    const uint32_t flags =
        furi_thread_flags_wait(INTERCOM_RX_THREAD_FLAGS_ALL, FuriFlagWaitAny, FuriWaitForever);

    furi_check((flags & FuriFlagError) == 0);

    if(flags & IntercomRxThreadFlagErrorOccurred) {
        furi_thread_suspend(furi_thread_get_current_id());
    }
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
        INTERCOM_RX_THREAD_NAME, INTERCOM_RX_THREAD_STACK_SIZE, intercom_rx_thread, instance);

    furi_state_subscribe(instance->state, intercom_rx_state_callback, rx_thread);

    furi_thread_start(rx_thread);
}
