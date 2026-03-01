#include "intercom_i.h"

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>

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

void intercom_dump_frame(const IntercomFrame* frame) {
    FuriString* tmp = furi_string_alloc();

    furi_string_printf(
        tmp,
        "chan : %hhu\r\n"
        "size : %hu\r\n"
        "data : \r\n",
        frame->channel_id,
        frame->data_size);

    for(uint32_t i = 0; i < sizeof(frame->data); ++i) {
        if(i && i % 32 == 0) furi_string_cat(tmp, "\r\n");
        furi_string_cat_printf(tmp, "%02X ", frame->data[i]);
    }

    furi_string_cat_printf(tmp, "\r\ncheck: 0x%04X\r\n", frame->check);

    furi_log_puts(furi_string_get_cstr(tmp));

    furi_string_free(tmp);
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
    furi_assert(context);

    Intercom* instance = context;

    if(instance->error_handling_disabled) {
        FURI_LOG_E(TAG, "Intercom error in disabled state: %d", error);
        return;
    }

#if defined(FURI_RAM_EXEC)
    FURI_LOG_E(TAG, "Intercom error: %d", error);
    return;
#endif

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

    const bool sync_result = intercom_sync_serial(instance->serial);

    if(sync_result) {
        furi_hal_serial_clear(instance->serial, FuriHalSerialDirectionTxRx);
        // TODO: find proper enterprise delay value
        furi_delay_ms(INTERCOM_MAGIC_DELAY);

        furi_check(furi_semaphore_release(instance->tx_semaphore) == FuriStatusOk);
        intercom_publish_sync_state_change(instance, true);

    } else {
        intercom_error_handler(IntercomErrorSync, instance);
    }

    return sync_result;
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

IntercomFrame* intercom_do_acquire_tx(Intercom* intercom) {
    furi_assert(intercom);
    furi_check(furi_semaphore_acquire(intercom->tx_semaphore, FuriWaitForever) == FuriStatusOk);
    return &intercom->tx_frame;
}

void intercom_do_tx(Intercom* intercom) {
    furi_assert(intercom);
    furi_event_loop_set_custom_event(intercom->event_loop, IntercomCustomEventDataAvailable);
}

static void intercom_init_channels(Intercom* instance) {
    for(IntercomChannelId i = 0; i < IntercomChannelIdMax; i++) {
        intercom_channel_init(&instance->handles[i], instance);
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
    intercom_error_handling_enable(instance);

    // dont't hold up the rest of the system
    furi_record_create(RECORD_INTERCOM, instance);

    if(intercom_do_sync(instance)) {
        intercom_start_rx_thread(instance);
    }

    return instance;
}

size_t
    intercom_tx(IntercomChannel* channel, const void* data, size_t data_size, uint32_t timeout) {
    furi_check(channel);
    furi_check(data);
    furi_check(data_size > 0);

    Intercom* instance = channel->intercom;

    size_t sent_data_size = 0;
    const uint32_t timeout_ticks = furi_ms_to_ticks(timeout);
    const uint32_t start_time = furi_get_tick();
    const uint32_t end_by = start_time + timeout_ticks;

    if(!intercom_channel_await_peer_ready(channel, timeout_ticks)) return 0;

    while(furi_semaphore_acquire(instance->tx_semaphore, end_by - furi_get_tick()) ==
          FuriStatusOk) {
        IntercomFrame* frame = &instance->tx_frame;
        IntercomChannelId channel_id = channel - instance->handles;

        const size_t chunk_size = MIN(data_size - sent_data_size, sizeof(frame->data));

        memcpy(frame->data, data + sent_data_size, chunk_size);
        frame->data_size = chunk_size;
        frame->channel_id = channel_id;
        frame->check = intercom_frame_get_checksum(frame);

        INTERCOM_LOG_D("TX payload size: %zu byte(s)", data_size);
        intercom_do_tx(instance);

        sent_data_size += chunk_size;
        if(sent_data_size == data_size) break;
    }

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
    if(context) furi_check(callback);

    IntercomChannel* handle = &instance->handles[channel_id];
    intercom_channel_set_callback(handle, callback, context);
    intercom_channel_send_ready(handle);

    return handle;
}

void intercom_error_handling_enable(Intercom* instance) {
    furi_check(instance);
    instance->error_handling_disabled = false;
}

void intercom_error_handling_disable(Intercom* instance) {
    furi_check(instance);
    instance->error_handling_disabled = true;
}

FuriPubSub* intercom_get_pubsub(Intercom* instance) {
    furi_check(instance);
    return instance->pubsub;
}

bool intercom_is_in_sync(Intercom* instance) {
    furi_check(instance);
    return instance->is_in_sync;
}

int32_t intercom_srv(void* arg) {
    UNUSED(arg);
    // TODO: Find a better place for this code
#if defined(BSB_MCU_U5)
    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        furi_hal_power_reset_917(false);
        FURI_LOG_I(TAG, "917 was reset");
    }
#endif

    Intercom* instance = intercom_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
