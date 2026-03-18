#include "intercom_i.h"

#define TAG "IntercomSrv"

// Called in ISR context
static void intercom_serial_tx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxEvent event,
    void* context) {
    UNUSED(handle);
    furi_assert(context);

    Intercom* instance = context;

    if(event & FuriHalSerialTxEventComplete) {
        furi_event_loop_set_custom_event(instance->event_loop, IntercomCustomEventFrameSent);
    }
}

static void intercom_state_callback(const void* item, void* context) {
    UNUSED(item);
    furi_assert(context);

    Intercom* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, IntercomCustomEventStatusChanged);
}

static void intercom_startup_sequence(Intercom* instance) {
    IntercomStatus status;

#if defined(BSB_MCU_U5)
    intercom_reset_other_side();
#endif // BSB_MCU_U5

    if(intercom_sync_serial(instance->serial)) {
        status = IntercomStatusOk;
    } else {
        status = IntercomStatusErrorSync;
    }

    intercom_set_status(instance, status);
}

static void intercom_begin_operation(Intercom* instance) {
    furi_hal_serial_set_tx_callback(instance->serial, intercom_serial_tx_callback, instance);
    furi_hal_serial_dma_init(instance->serial);

    intercom_start_rx_thread(instance);
    // Begin serving API requests
    furi_check(furi_semaphore_release(instance->tx_semaphore) == FuriStatusOk);
}

static void intercom_unrecoverable_error(void) {
    FURI_LOG_E(TAG, "Unrecoverable error encountered. Suspending service...");
    furi_thread_suspend(furi_thread_get_current_id());
}

static FURI_ALWAYS_INLINE void intercom_process_status_changed_event(Intercom* instance) {
    IntercomStatus status;
    furi_state_get(instance->state, &status);

    if(status == IntercomStatusUnknown) {
        intercom_startup_sequence(instance);

    } else if(status == IntercomStatusOk) {
        intercom_begin_operation(instance);

    } else if(status == IntercomStatusErrorSync) {
        FURI_LOG_E(TAG, "Failed to sync with the other side");
        intercom_unrecoverable_error();

    } else if(status == IntercomStatusErrorFraming) {
        FURI_LOG_E(TAG, "Corrupt frame received");
        intercom_unrecoverable_error();

    } else if(status == IntercomStatusErrorTimeout) {
        FURI_LOG_E(TAG, "Other side is not responding");
        intercom_unrecoverable_error();

    } else {
        furi_crash("Invalid IntercomStatus value");
    }
}

static FURI_ALWAYS_INLINE void intercom_process_tx_frame_sent_event(Intercom* instance) {
    furi_event_loop_timer_stop(instance->tx_timer);
    furi_semaphore_release(instance->tx_semaphore);

    INTERCOM_LOG_D("Frame transmit complete");
}

static FURI_ALWAYS_INLINE void intercom_process_tx_data_available_event(Intercom* instance) {
    IntercomFrame* tx_frame = &instance->tx_frame;

    furi_event_loop_timer_start(instance->tx_timer, INTERCOM_TX_TIMEOUT_MS);
    furi_hal_serial_dma_tx(instance->serial, (void*)tx_frame, sizeof(IntercomFrame));
}

static void intercom_custom_event_callback(uint32_t events, void* context) {
    Intercom* instance = context;
    if(events & IntercomCustomEventStatusChanged) {
        INTERCOM_LOG_D("IntercomCustomEventStatusChanged");
        intercom_process_status_changed_event(instance);
    }
    if(events & IntercomCustomEventFrameSent) {
        INTERCOM_LOG_D("IntercomCustomEventFrameSent");
        intercom_process_tx_frame_sent_event(instance);
    }
    if(events & IntercomCustomEventDataAvailable) {
        INTERCOM_LOG_D("IntercomCustomEventDataAvailable");
        intercom_process_tx_data_available_event(instance);
    }
}

static void intercom_tx_timer_callback(void* context) {
    furi_assert(context);

    Intercom* instance = context;
    intercom_set_status(instance, IntercomStatusErrorTimeout);
}

static void intercom_init_channels(Intercom* instance) {
    for(IntercomChannelId i = 0; i < IntercomChannelIdMax; i++) {
        intercom_channel_init(&instance->channels[i], instance);
    }
}

static Intercom* intercom_alloc(void) {
    Intercom* instance = malloc(sizeof(Intercom));

    instance->event_loop = furi_event_loop_alloc();
    instance->tx_semaphore = furi_semaphore_alloc(1, 0);
    instance->tx_timer = furi_event_loop_timer_alloc(
        instance->event_loop, intercom_tx_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->serial = furi_hal_serial_control_acquire(INTERCOM_SERIAL);
    instance->state = furi_state_alloc(sizeof(IntercomStatus));

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, intercom_custom_event_callback, instance);

    furi_hal_serial_init(instance->serial, INTERCOM_BAUD_RATE);
    furi_hal_serial_set_hw_flow_control(instance->serial, FuriHalSerialHwFlowControlRtsCts);

    intercom_init_channels(instance);

    intercom_set_status(instance, IntercomStatusUnknown);
    furi_state_subscribe(instance->state, intercom_state_callback, instance);

    // dont't hold up the rest of the system
    furi_record_create(RECORD_INTERCOM, instance);

    return instance;
}

// Private API

void intercom_set_status(Intercom* instance, IntercomStatus new_status) {
    with_furi_state(instance->state, IntercomStatus * status, { *status = new_status; });
}

size_t intercom_tx_internal(
    Intercom* instance,
    IntercomChannelId channel_id,
    const void* data,
    size_t data_size,
    uint32_t timeout_ticks) {
    if(furi_semaphore_acquire(instance->tx_semaphore, timeout_ticks) != FuriStatusOk) {
        return 0;
    }

    const size_t tx_size = intercom_build_frame(&instance->tx_frame, channel_id, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, IntercomCustomEventDataAvailable);

    INTERCOM_LOG_D("TX payload size: %zu byte(s)", tx_size);

    return tx_size;
}

// Public API

size_t
    intercom_tx(IntercomChannel* channel, const void* data, size_t data_size, uint32_t timeout) {
    furi_check(channel);
    furi_check(data);
    furi_check(data_size > 0);

    const uint32_t timeout_ticks = furi_ms_to_ticks(timeout);
    const uint32_t deadline_ticks = furi_get_tick() + timeout_ticks;

    if(!intercom_channel_await_peer_ready(channel, timeout_ticks)) {
        return 0;
    }

    Intercom* instance = channel->intercom;
    const IntercomChannelId channel_id = channel - instance->channels;

    size_t sent_data_size = 0;

    do {
        const void* data_remain = data + sent_data_size;
        const size_t data_size_remain = data_size - sent_data_size;
        const uint32_t timeout_ticks_left = deadline_ticks - furi_get_tick();

        const size_t chunk_size = intercom_tx_internal(
            instance, channel_id, data_remain, data_size_remain, timeout_ticks_left);

        if(chunk_size == 0) {
            break;
        }

        sent_data_size += chunk_size;

    } while(sent_data_size != data_size);

    return sent_data_size;
}

IntercomChannel* intercom_channel_open(
    Intercom* instance,
    IntercomChannelId channel_id,
    IntercomRxCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(channel_id < IntercomChannelIdMax);
    furi_check(channel_id != IntercomChannelIdMeta);

    IntercomChannel* channel = &instance->channels[channel_id];
    intercom_channel_set_callback(channel, callback, context);
    intercom_channel_send_ready(channel);

    return channel;
}

FuriState* intercom_get_state(const Intercom* instance) {
    furi_check(instance);
    return instance->state;
}

// Service thread

int32_t intercom_srv(void* arg) {
    UNUSED(arg);

    Intercom* instance = intercom_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
