#include "intercom_i.h"

#define TAG "IntercomSrv"

#define INTERCOM_TX_TIMEOUT_MS (1000UL)

#ifdef INTERCOM_DEBUG
#define INTERCOM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INTERCOM_LOG_D(...)
#endif

#ifndef INTERCOM_BAUD_RATE
#define INTERCOM_BAUD_RATE (11250000UL)
#endif

#if defined(STM32U595xx)
#define TARGET_F20
#elif defined(SI917)
#define TARGET_F64
#else
#error "Unsupported MCU"
#endif

#if defined(TARGET_F20)
#define INTERCOM_SERIAL FuriHalSerialIdUsart1
#define INTERCOM_GPIO   gpio_917_irq
#elif defined(TARGET_F64)
#define INTERCOM_SERIAL FuriHalSerialIdUsart0
#define INTERCOM_GPIO   gpio_u5_irq
#else
#error "Unsupported target"
#endif

typedef struct {
    IntercomRxCallback rx_callback;
    void* callback_context;
} IntercomChannelData;

struct Intercom {
    FuriEventLoop* event_loop;
    FuriSemaphore* tx_semaphore;
    FuriEventLoopTimer* tx_timer;
    FuriHalSerialHandle* serial;
    IntercomChannelData channels[IntercomChannelMax];
    IntercomFrame tx_frame;
    IntercomFrame rx_frame;
    IntercomErrorCallback error_callback;
    void* error_callback_context;
};

typedef enum {
    IntercomEventSyncRequested = 1UL << 0,
    IntercomEventFrameSent = 1UL << 1,
    IntercomEventFrameReceived = 1UL << 2,
    IntercomEventDataAvailable = 1UL << 3,
} IntercomEvent;

// Called in ISR context
static void intercom_gpio_irq_callback(void* context) {
    furi_assert(context);
    Intercom* instance = context;

    furi_hal_gpio_remove_int_callback(&INTERCOM_GPIO);
    furi_event_loop_set_custom_event(instance->event_loop, IntercomEventSyncRequested);
}

// Called in ISR context
static void intercom_serial_tx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxEvent event,
    void* context) {
    UNUSED(handle);

    Intercom* instance = context;

    if(event & FuriHalSerialTxEventComplete) {
        furi_event_loop_set_custom_event(instance->event_loop, IntercomEventFrameSent);
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
        furi_event_loop_set_custom_event(instance->event_loop, IntercomEventFrameReceived);
    }
}

static void intercom_dump_frame(const IntercomFrame* frame) {
    FuriString* tmp = furi_string_alloc();

    furi_string_printf(
        tmp,
        "chan : %hhu\r\n"
        "size : %hu\r\n"
        "data : \r\n",
        frame->channel,
        frame->data_size);

    for(uint32_t i = 0; i < sizeof(frame->data); ++i) {
        if(i && i % 32 == 0) furi_string_cat(tmp, "\r\n");
        furi_string_cat_printf(tmp, "%02X ", frame->data[i]);
    }

    furi_string_cat_printf(tmp, "\r\ncheck: 0x%04X", frame->check);

    furi_log_puts(furi_string_get_cstr(tmp));

    furi_string_free(tmp);
}

static void intercom_default_error_callback(IntercomError error, void* context) {
    furi_assert(context);

    Intercom* instance = context;

    if(error == IntercomErrorSync) {
        furi_crash("Other side not in sync");

    } else if(error == IntercomErrorFraming) {
        intercom_dump_frame(&instance->rx_frame);
        furi_crash("Corrupted frame received");

    } else if(error == IntercomErrorTransmit) {
        furi_crash("Other side has died");

    } else {
        furi_crash();
    }
}

static FURI_ALWAYS_INLINE void intercom_process_sync_requested_event(Intercom* instance) {
    INTERCOM_LOG_D("Sync requested");

    furi_hal_serial_dma_rx_stop(instance->serial);

    if(intercom_sync_serial(instance->serial)) {
// TODO: Unify function signatures
#if defined(TARGET_F20)
        furi_hal_gpio_init_simple(&INTERCOM_GPIO, GpioModeInterruptFall);
        furi_hal_gpio_add_int_callback(&INTERCOM_GPIO, intercom_gpio_irq_callback, instance);
#elif defined(TARGET_F64)
        furi_hal_gpio_add_int_callback(
            &INTERCOM_GPIO, GpioConditionFall, intercom_gpio_irq_callback, instance);
#else
#error "Unsupported target"
#endif
        furi_hal_serial_clear(instance->serial, FuriHalSerialDirectionTxRx);
        furi_hal_serial_dma_rx_start(
            instance->serial, (void*)&instance->rx_frame, sizeof(IntercomFrame));
    } else {
        instance->error_callback(IntercomErrorSync, instance->error_callback_context);
    }
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

    IntercomFrame* rx_frame = &instance->rx_frame;

    if(intercom_frame_is_valid(rx_frame)) {
        const IntercomChannelData* channel_data = &instance->channels[rx_frame->channel];

        if(channel_data->rx_callback) {
            channel_data->rx_callback(
                rx_frame->data, rx_frame->data_size, channel_data->callback_context);
        }
    } else {
        instance->error_callback(IntercomErrorFraming, instance->error_callback_context);
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
    if(events & IntercomEventSyncRequested) {
        INTERCOM_LOG_D("IntercomEventSyncRequested");
        intercom_process_sync_requested_event(instance);
    }
    if(events & IntercomEventFrameSent) {
        INTERCOM_LOG_D("IntercomEventFrameSent");
        intercom_process_tx_frame_event(instance);
    }
    if(events & IntercomEventFrameReceived) {
        INTERCOM_LOG_D("IntercomEventFrameReceived");
        intercom_process_rx_frame_event(instance);
    }
    if(events & IntercomEventDataAvailable) {
        INTERCOM_LOG_D("IntercomEventData");
        intercom_process_tx_data_event(instance);
    }
}

static void intercom_tx_timer_callback(void* context) {
    furi_assert(context);

    Intercom* instance = context;
    instance->error_callback(IntercomErrorTransmit, instance->error_callback_context);
}

static Intercom* intercom_alloc(void) {
    Intercom* instance = malloc(sizeof(Intercom));

    instance->event_loop = furi_event_loop_alloc();
    instance->tx_semaphore = furi_semaphore_alloc(1, 1);
    instance->tx_timer = furi_event_loop_timer_alloc(
        instance->event_loop, intercom_tx_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->serial = furi_hal_serial_control_acquire(INTERCOM_SERIAL);
    instance->error_callback = intercom_default_error_callback;
    instance->error_callback_context = instance;

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, intercom_custom_event_callback, instance);

    furi_hal_serial_init(instance->serial, INTERCOM_BAUD_RATE);
    furi_hal_serial_set_hw_flow_control(instance->serial, FuriHalSerialHwFlowControlRtsCts);
    furi_hal_serial_set_callback(
        instance->serial, intercom_serial_tx_callback, intercom_serial_rx_callback, instance);
    // Pulse gpio_xxx_irq pin to request synchronisation procedure
    intercom_sync_request(&INTERCOM_GPIO);
    // Perform initial synchronisation procedure
    intercom_process_sync_requested_event(instance);

    furi_record_create(RECORD_INTERCOM, instance);
    return instance;
}

size_t intercom_tx(
    Intercom* instance,
    IntercomChannel channel,
    const void* data,
    size_t data_size,
    uint32_t timeout) {
    furi_check(instance);
    furi_check(data);
    furi_check(data_size > 0);

    size_t sent_data_size = 0;

    const uint32_t start_time = furi_get_tick();
    uint32_t remaining_time = timeout;

    while(furi_semaphore_acquire(instance->tx_semaphore, remaining_time) == FuriStatusOk) {
        IntercomFrame* frame = &instance->tx_frame;

        const size_t chunk_size = MIN(data_size - sent_data_size, sizeof(frame->data));

        memcpy(frame->data, data + sent_data_size, chunk_size);
        frame->data_size = chunk_size;

        frame->channel = channel;
        frame->check = intercom_frame_get_checksum(frame);

        INTERCOM_LOG_D("TX payload size: %zu byte(s)", data_size);
        furi_event_loop_set_custom_event(instance->event_loop, IntercomEventDataAvailable);

        sent_data_size += chunk_size;
        if(sent_data_size == data_size) break;

        const uint32_t elapsed_time = furi_get_tick() - start_time;
        if(elapsed_time >= remaining_time) break;

        remaining_time -= elapsed_time;
    }

    return sent_data_size;
}

void intercom_set_rx_callback(
    Intercom* instance,
    IntercomChannel channel,
    IntercomRxCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(channel < COUNT_OF(instance->channels));

    IntercomChannelData* port_data = &instance->channels[channel];

    port_data->rx_callback = callback;
    port_data->callback_context = context;
}

void intercom_set_error_callback(Intercom* instance, IntercomErrorCallback callback, void* context) {
    furi_check(instance);

    if(callback) {
        instance->error_callback = callback;
        instance->error_callback_context = context;
    } else {
        instance->error_callback = intercom_default_error_callback;
        instance->error_callback_context = instance;
    }
}

int32_t intercom_srv(void* arg) {
    UNUSED(arg);

    Intercom* instance = intercom_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
