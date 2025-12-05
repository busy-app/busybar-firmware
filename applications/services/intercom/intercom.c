#include "intercom_i.h"
#include <furi_hal_nvm.h>
#include <furi_hal_power.h>

#define TAG "IntercomSrv"

#define INTERCOM_TX_TIMEOUT_MS                 (1000UL)
#define INTERCOM_INITIAL_SYNC_RETRY_LOCKOUT_MS (500UL)

#ifdef INTERCOM_DEBUG
#define INTERCOM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INTERCOM_LOG_D(...)
#endif

#ifndef INTERCOM_BAUD_RATE
#define INTERCOM_BAUD_RATE (11250000UL)
#endif

#if !defined(BSB_MCU_SI917) && !defined(BSB_MCU_U5)
#error "Unsupported MCU"
#endif

#if defined(BSB_MCU_U5)
#define INTERCOM_SERIAL FuriHalSerialIdUsart1
#define INTERCOM_GPIO   gpio_917_irq
#elif defined(BSB_MCU_SI917)
#define INTERCOM_SERIAL FuriHalSerialIdUsart0
#define INTERCOM_GPIO   gpio_u5_irq
#else
#error "Unsupported target"
#endif

#define INTERCOM_MAGIC_DELAY (100UL)

typedef enum {
    IntercomCustomEventSyncRequested = 1UL << 0,
    IntercomCustomEventFrameSent = 1UL << 1,
    IntercomCustomEventDataAvailable = 1UL << 2,
} IntercomCustomEvent;

typedef enum {
    IntercomThreadFlagFrameReceived = 1UL << 0,
    IntercomThreadFlagSyncStarted = 1UL << 1,
    IntercomThreadFlagSyncFinished = 1UL << 2,
} IntercomThreadFlag;

#define INTERCOM_THREAD_FLAGS_ALL                                      \
    (IntercomThreadFlagFrameReceived | IntercomThreadFlagSyncStarted | \
     IntercomThreadFlagSyncFinished)

// Called in ISR context
static void intercom_gpio_irq_callback(void* context) {
    furi_assert(context);
    Intercom* instance = context;

    furi_hal_gpio_remove_int_callback(&INTERCOM_GPIO);
    furi_event_loop_set_custom_event(instance->event_loop, IntercomCustomEventSyncRequested);
}

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

// Called in ISR context
static void intercom_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    UNUSED(handle);

    Intercom* instance = context;

    if(event & FuriHalSerialRxEventData) {
        furi_thread_flags_set(instance->rx_thread, IntercomThreadFlagFrameReceived);
    }
}

static void intercom_dump_frame(const IntercomFrame* frame) {
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

    furi_string_cat_printf(tmp, "\r\ncheck: 0x%04X", frame->check);

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

static bool intercom_try_sync(Intercom* instance) {
    INTERCOM_LOG_D("Sync requested");

    FuriStatus expected_acq_status = instance->is_initial_sync_done ? FuriStatusOk :
                                                                      FuriStatusErrorResource;
    furi_check(furi_semaphore_acquire(instance->tx_semaphore, 0) == expected_acq_status);

    furi_thread_flags_set(furi_thread_get_id(instance->rx_thread), IntercomThreadFlagSyncStarted);
    furi_check(furi_semaphore_acquire(instance->sync_semaphore, FuriWaitForever) == FuriStatusOk);

    bool result = intercom_sync_serial(instance->serial);
    if(result) {
        // TODO: Unify function signatures
#if defined(BSB_MCU_U5)
        furi_hal_gpio_init_simple(&INTERCOM_GPIO, GpioModeInterruptFall);
        furi_hal_gpio_add_int_callback(&INTERCOM_GPIO, intercom_gpio_irq_callback, instance);
#elif defined(BSB_MCU_SI917)
        furi_hal_gpio_add_int_callback(
            &INTERCOM_GPIO, GpioConditionFall, intercom_gpio_irq_callback, instance);
#else
#error "Unsupported target"
#endif
        furi_hal_serial_clear(instance->serial, FuriHalSerialDirectionTxRx);

        intercom_publish_sync_state_change(instance, true);
        instance->is_initial_sync_done = true;

        // TODO: find proper enterprise delay value
        furi_delay_ms(INTERCOM_MAGIC_DELAY);
        furi_check(furi_semaphore_release(instance->tx_semaphore) == FuriStatusOk);
    } else {
        if(instance->is_initial_sync_done) {
            intercom_error_handler(IntercomErrorSync, instance);
        }
    }

    furi_thread_flags_set(furi_thread_get_id(instance->rx_thread), IntercomThreadFlagSyncFinished);
    furi_check(furi_semaphore_acquire(instance->sync_semaphore, FuriWaitForever) == FuriStatusOk);

    return result;
}

static FURI_ALWAYS_INLINE void intercom_send_tx_frame(Intercom* instance) {
    IntercomFrame* tx_frame = &instance->tx_frame;

    furi_event_loop_timer_start(instance->tx_timer, INTERCOM_TX_TIMEOUT_MS);
    furi_hal_serial_dma_tx(instance->serial, (void*)tx_frame, sizeof(IntercomFrame));
}

static FURI_ALWAYS_INLINE void intercom_process_tx_data_event(Intercom* instance) {
    intercom_send_tx_frame(instance);
}

static FURI_ALWAYS_INLINE void intercom_process_rx_frame_event(Intercom* instance) {
    INTERCOM_LOG_D("Frame received");

    const IntercomFrame* rx_frame = &instance->rx_frame;

    if(intercom_frame_is_valid(rx_frame)) {
        IntercomChannelId channel_id = rx_frame->channel_id;
        IntercomChannel* channel = &instance->handles[channel_id];
        intercom_channel_call_callback(channel, rx_frame);

    } else {
        intercom_error_handler(IntercomErrorFraming, instance);
    }

    furi_hal_serial_dma_rx_start(
        instance->serial, (void*)&instance->rx_frame, sizeof(IntercomFrame));
}

static FURI_ALWAYS_INLINE void intercom_process_tx_frame_event(Intercom* instance) {
    furi_event_loop_timer_stop(instance->tx_timer);
    furi_semaphore_release(instance->tx_semaphore);

    INTERCOM_LOG_D("Frame transmit complete");
}

static void intercom_custom_event_callback(uint32_t events, void* context) {
    Intercom* instance = context;
    if(events & IntercomCustomEventSyncRequested) {
        INTERCOM_LOG_D("IntercomCustomEventSyncRequested");
        intercom_try_sync(instance);
    }
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

static int32_t intercom_rx_thread(void* arg) {
    furi_assert(arg);
    Intercom* instance = arg;

    for(;;) {
        const uint32_t flags =
            furi_thread_flags_wait(INTERCOM_THREAD_FLAGS_ALL, FuriFlagWaitAny, FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0);

        if(flags & IntercomThreadFlagFrameReceived) {
            INTERCOM_LOG_D("IntercomThreadFlagFrameReceived");
            intercom_process_rx_frame_event(instance);
        }
        if(flags & IntercomThreadFlagSyncStarted) {
            INTERCOM_LOG_D("IntercomThreadFlagSyncStarted");
            furi_hal_serial_dma_rx_stop(instance->serial);
            furi_check(furi_semaphore_release(instance->sync_semaphore) == FuriStatusOk);
        }
        if(flags & IntercomThreadFlagSyncFinished) {
            INTERCOM_LOG_D("IntercomThreadFlagSyncFinished");
            furi_hal_serial_dma_rx_start(
                instance->serial, (void*)&instance->rx_frame, sizeof(IntercomFrame));
            furi_check(furi_semaphore_release(instance->sync_semaphore) == FuriStatusOk);
        }
    }

    return 0;
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

static Intercom* intercom_alloc(void) {
    Intercom* instance = malloc(sizeof(Intercom));

    instance->rx_thread =
        furi_thread_alloc_service("IntercomRxSrv", 2048, intercom_rx_thread, instance);
    instance->sync_semaphore = furi_semaphore_alloc(1, 0);
    instance->event_loop = furi_event_loop_alloc();
    instance->tx_semaphore = furi_semaphore_alloc(1, 0);
    instance->tx_timer = furi_event_loop_timer_alloc(
        instance->event_loop, intercom_tx_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->serial = furi_hal_serial_control_acquire(INTERCOM_SERIAL);
    instance->pubsub = furi_pubsub_alloc();
    intercom_error_handling_enable(instance);

    for(IntercomChannelId i = 0; i < IntercomChannelIdMax; i++) {
        intercom_channel_init(&instance->handles[i], instance);
    }

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, intercom_custom_event_callback, instance);

    furi_hal_serial_init(instance->serial, INTERCOM_BAUD_RATE);
    furi_hal_serial_set_hw_flow_control(instance->serial, FuriHalSerialHwFlowControlRtsCts);
    furi_hal_serial_set_callback(
        instance->serial, intercom_serial_tx_callback, intercom_serial_rx_callback, instance);
    furi_thread_start(instance->rx_thread);

    // dont't hold up the rest of the system
    furi_record_create(RECORD_INTERCOM, instance);

    while(1) {
        intercom_sync_request(&INTERCOM_GPIO);
        if(intercom_try_sync(instance)) break;

        FURI_LOG_E(
            TAG,
            "Initial sync failed, retrying in %ld ms",
            INTERCOM_INITIAL_SYNC_RETRY_LOCKOUT_MS);
        furi_delay_ms(INTERCOM_INITIAL_SYNC_RETRY_LOCKOUT_MS);
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
