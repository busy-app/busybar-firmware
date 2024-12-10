#include "intercom.h"

#include <furi.h>

#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#include "intercom_frame.h"

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

#ifdef STM32U595xx
#define INTERCOM_SERIAL FuriHalSerialIdUsart1
#else
#define INTERCOM_SERIAL FuriHalSerialIdUsart0
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
};

typedef enum {
    IntercomEventData = 1UL << 0,
    IntercomEventFrameSent = 1UL << 1,
    IntercomEventFrameReceived = 1UL << 2,
} IntercomEvent;

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

static FURI_ALWAYS_INLINE void intercom_send_tx_frame(Intercom* instance) {
    IntercomFrame* tx_frame = &instance->tx_frame;

    furi_hal_serial_dma_tx(instance->serial, (void*)tx_frame, sizeof(IntercomFrame));
    furi_event_loop_timer_start(instance->tx_timer, INTERCOM_TX_TIMEOUT_MS);
}

static FURI_ALWAYS_INLINE void intercom_process_tx_data_event(Intercom* instance) {
    intercom_send_tx_frame(instance);
}

static FURI_ALWAYS_INLINE void intercom_process_rx_frame_event(Intercom* instance) {
    INTERCOM_LOG_D("Frame received");

    const IntercomFrame* rx_frame = &instance->rx_frame;

    if(!intercom_frame_is_valid(rx_frame)) {
        intercom_dump_frame(rx_frame);
        furi_delay_ms(10);
        furi_crash("Corrupted frame received");
    }

    const IntercomChannelData* channel_data = &instance->channels[rx_frame->channel];

    if(channel_data->rx_callback) {
        channel_data->rx_callback(
            rx_frame->data, rx_frame->data_size, channel_data->callback_context);
    }

    furi_hal_serial_dma_rx_start(instance->serial, (void*)rx_frame, sizeof(IntercomFrame));
}

static FURI_ALWAYS_INLINE void intercom_process_tx_frame_event(Intercom* instance) {
    furi_event_loop_timer_stop(instance->tx_timer);
    furi_semaphore_release(instance->tx_semaphore);

    INTERCOM_LOG_D("Frame transmit complete");
}

static void intercom_custom_event_callback(uint32_t events, void* context) {
    Intercom* instance = context;
    if(events & IntercomEventData) {
        INTERCOM_LOG_D("IntercomEventData");
        intercom_process_tx_data_event(instance);
    }
    if(events & IntercomEventFrameReceived) {
        INTERCOM_LOG_D("IntercomEventFrameReceived");
        intercom_process_rx_frame_event(instance);
    }
    if(events & IntercomEventFrameSent) {
        INTERCOM_LOG_D("IntercomEventFrameSent");
        intercom_process_tx_frame_event(instance);
    }
}

static void intercom_tx_timer_callback(void* context) {
    Intercom* instance = context;
    UNUSED(instance);
    // TODO: This is an unrecoverable situation, reboot both sides
    furi_crash("Other side has died");
}

static bool intercom_wait_for_serial_sync(FuriHalSerialHandle* serial) {
    bool success = false;

    const uint8_t leader = 0xAA;

    // TODO: Wait timeout
    while(true) {
        // Listen for leader first, then transmit ourselves
        if(furi_hal_serial_rx_available(serial)) {
            if(furi_hal_serial_rx(serial) == leader) {
                success = true;
            }
        }
        // Ensure that the other side receives at least one
        // leader character when configured properly
        furi_hal_serial_tx(serial, &leader, 1);
        furi_hal_serial_tx_wait_complete(serial);

        if(success) break;

        furi_delay_ms(10);
    }

    return success;
}

static Intercom* intercom_alloc(void) {
    Intercom* instance = malloc(sizeof(Intercom));

    instance->event_loop = furi_event_loop_alloc();
    instance->tx_semaphore = furi_semaphore_alloc(1, 1);
    instance->tx_timer = furi_event_loop_timer_alloc(
        instance->event_loop, intercom_tx_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->serial = furi_hal_serial_control_acquire(INTERCOM_SERIAL);
    furi_hal_serial_init(instance->serial, INTERCOM_BAUD_RATE);
    furi_hal_serial_set_hw_flow_control(instance->serial, FuriHalSerialHwFlowControlRtsCts);
    furi_hal_serial_set_callback(
        instance->serial, intercom_serial_tx_callback, intercom_serial_rx_callback, instance);
    furi_event_loop_set_custom_event_callback(
        instance->event_loop, intercom_custom_event_callback, instance);

    intercom_wait_for_serial_sync(instance->serial);

    furi_hal_serial_clear(instance->serial, FuriHalSerialDirectionTxRx);

    furi_hal_serial_dma_rx_start(
        instance->serial, (void*)&instance->rx_frame, sizeof(IntercomFrame));

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

        INTERCOM_LOG_D("TX payload size: %zu byte(s)", payload_size);
        furi_event_loop_set_custom_event(instance->event_loop, IntercomEventData);

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
    furi_check(callback);
    furi_check(channel < COUNT_OF(instance->channels));

    IntercomChannelData* port_data = &instance->channels[channel];
    furi_check(port_data->rx_callback == NULL);

    port_data->rx_callback = callback;
    port_data->callback_context = context;
}

int32_t intercom_srv(void* arg) {
    UNUSED(arg);

    Intercom* instance = intercom_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
