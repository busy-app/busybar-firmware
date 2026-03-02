#include "intercom_i.h"

#define TAG "IntercomSrv"

// Called in ISR context
static void intercom_serial_tx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxEvent event,
    void* context) {
    UNUSED(handle);

    Intercom* instance = context;

    if(event & FuriHalSerialTxEventComplete) {
        furi_event_loop_set_custom_event(instance->event_loop, IntercomCustomEventFrameSent);
    }
}

static void intercom_publish_sync_state_change(Intercom* instance, bool is_in_sync) {
    /* Repetitive desyncs are useless, while resyncs can be used in some contexts. */
    if(is_in_sync || instance->is_in_sync) {
        IntercomEvent pubsub_message = {
            .type = IntercomEventTypeSyncStateChanged,
            .is_in_sync = is_in_sync,
        };
        furi_pubsub_publish(instance->pubsub, &pubsub_message);
        instance->is_in_sync = is_in_sync;
    }
}

static void intercom_unrecoverable_error(Intercom* instance, const char* message) {
    while(true) {
        IntercomEvent pubsub_message = {
            .type = IntercomEventTypeError,
            .message = message,
        };

        furi_pubsub_publish(instance->pubsub, &pubsub_message);
        FURI_LOG_E(TAG, message);
        furi_delay_ms(5000);
    }
}

static void intercom_error_handler(IntercomError error, void* context) {
#if defined(FURI_RAM_EXEC)
    FURI_LOG_E(TAG, "Intercom error: %d", error);
    return;
#endif

    furi_assert(context);
    Intercom* instance = context;

    if(error == IntercomErrorSync) {
        intercom_publish_sync_state_change(instance, false);
        intercom_unrecoverable_error(instance, "Externally requested sync failed");
    } else if(error == IntercomErrorFraming) {
        intercom_dump_frame(&instance->rx_frame);
        intercom_publish_sync_state_change(instance, false);
        intercom_unrecoverable_error(instance, "Corrupted frame received");
    } else if(error == IntercomErrorTransmit) {
        intercom_publish_sync_state_change(instance, false);
        intercom_unrecoverable_error(instance, "Other side has died");
    } else {
        intercom_publish_sync_state_change(instance, false);
        intercom_unrecoverable_error(instance, "Unknown error");
    }
}

static bool intercom_do_sync(Intercom* instance) {
    INTERCOM_LOG_D("Sync started");

    intercom_reset_other_side();

    const bool sync_success = intercom_sync_serial(instance->serial);

    if(sync_success) {
        furi_hal_serial_clear(instance->serial, FuriHalSerialDirectionTxRx);
        // TODO: find proper enterprise delay value
        furi_delay_ms(INTERCOM_MAGIC_DELAY);

        furi_check(furi_semaphore_release(instance->tx_semaphore) == FuriStatusOk);
        intercom_publish_sync_state_change(instance, true);

    } else {
        intercom_error_handler(IntercomErrorSync, instance);
    }

    return sync_success;
}

static FURI_ALWAYS_INLINE void intercom_send_tx_frame(Intercom* instance) {
    IntercomFrame* tx_frame = &instance->tx_frame;

    furi_event_loop_timer_start(instance->tx_timer, INTERCOM_TX_TIMEOUT_MS);
    furi_hal_serial_dma_tx(instance->serial, (void*)tx_frame, sizeof(IntercomFrame));
}

static FURI_ALWAYS_INLINE void intercom_process_tx_data_event(Intercom* instance) {
    intercom_send_tx_frame(instance);
}

static FURI_ALWAYS_INLINE void intercom_process_tx_frame_event(Intercom* instance) {
    furi_event_loop_timer_stop(instance->tx_timer);
    furi_semaphore_release(instance->tx_semaphore);

    INTERCOM_LOG_D("Frame transmit complete");
}

static void intercom_custom_event_callback(uint32_t events, void* context) {
    Intercom* instance = context;
    if(events & IntercomCustomEventFrameSent) {
        INTERCOM_LOG_D("IntercomCustomEventFrameSent");
        intercom_process_tx_frame_event(instance);
    }
    if(events & IntercomCustomEventDataAvailable) {
        INTERCOM_LOG_D("IntercomCustomEventDataAvailable");
        intercom_process_tx_data_event(instance);
    }
}

static void intercom_tx_timer_callback(void* context) {
    furi_assert(context);

    Intercom* instance = context;
    intercom_error_handler(IntercomErrorTransmit, instance);
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
    instance->pubsub = furi_pubsub_alloc();

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, intercom_custom_event_callback, instance);

    furi_hal_serial_init(instance->serial, INTERCOM_BAUD_RATE);
    furi_hal_serial_set_hw_flow_control(instance->serial, FuriHalSerialHwFlowControlRtsCts);
    furi_hal_serial_set_tx_callback(instance->serial, intercom_serial_tx_callback, instance);

    intercom_init_channels(instance);

    // dont't hold up the rest of the system
    furi_record_create(RECORD_INTERCOM, instance);

    if(intercom_do_sync(instance)) {
        intercom_start_rx_thread(instance);
    }

    return instance;
}

// Private API

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
    // TODO: Store channel id within the channel?
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

FuriPubSub* intercom_get_pubsub(Intercom* instance) {
    furi_check(instance);
    return instance->pubsub;
}

bool intercom_is_in_sync(Intercom* instance) {
    furi_check(instance);
    return instance->is_in_sync;
}

// Service thread

int32_t intercom_srv(void* arg) {
    UNUSED(arg);

    Intercom* instance = intercom_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
