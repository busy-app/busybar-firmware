#include "intercom.h"

#include <furi.h>

#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#include "intercom_frame.h"

#define TAG "IntercomSrv"

#ifdef INTERCOM_DEBUG
#define INTERCOM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INTERCOM_LOG_D(...)
#endif

#ifndef INTERCOM_BAUD_RATE
#define INTERCOM_BAUD_RATE (11250000UL)
#endif

// TODO: Reduce timeout to absolute minimum
#define INTERCOM_RESPONSE_TIMEOUT_MS (1000U)

#ifdef STM32U595xx
#define INTERCOM_SERIAL FuriHalSerialIdUsart1
#else
#define INTERCOM_SERIAL FuriHalSerialIdUsart0
#endif

typedef enum {
    IntercomStateIdle,
    IntercomStateWaitingConfirmation,
} IntercomState;

typedef enum {
    IntercomEventData = 1UL << 0,
    IntercomEventFrameSent = 1UL << 1,
    IntercomEventFrameReceived = 1UL << 2,
} IntercomEvent;

struct Intercom {
    FuriEventLoop* event_loop;
    FuriSemaphore* tx_semaphore;
    FuriEventLoopTimer* response_timer;
    FuriHalSerialHandle* serial;
    IntercomRxCallback rx_callback;
    void* callback_context;
    IntercomFrame tx_frame;
    IntercomFrame rx_frame;
    IntercomState state;
};

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

static FURI_ALWAYS_INLINE void intercom_send_data_frame(Intercom* instance) {
    IntercomFrame* tx_frame = &instance->tx_frame;

    tx_frame->header.id = 0;
    tx_frame->header.type = IntercomFrameTypeData;
    tx_frame->d.trailer.check = intercom_frame_calculate_checksum(tx_frame);

    // Send request
    furi_hal_serial_dma_tx(instance->serial, (uint8_t*)tx_frame, INTERCOM_D_FRAME_SIZE);
    furi_hal_serial_dma_rx_start(
        instance->serial, (uint8_t*)&instance->rx_frame, INTERCOM_S_FRAME_SIZE);
}

static FURI_ALWAYS_INLINE void intercom_send_service_frame(
    Intercom* instance,
    IntercomFrameError error,
    uint16_t expected_size) {
    IntercomFrame* tx_frame = &instance->tx_frame;

    tx_frame->header.id = 0;
    tx_frame->header.type = IntercomFrameTypeService;
    tx_frame->header.error = error;
    tx_frame->s.trailer.check = intercom_frame_calculate_checksum(tx_frame);

    // Send confirmation
    furi_hal_serial_dma_tx(instance->serial, (uint8_t*)tx_frame, INTERCOM_S_FRAME_SIZE);
    furi_hal_serial_dma_rx_start(instance->serial, (uint8_t*)&instance->rx_frame, expected_size);
}

static FURI_ALWAYS_INLINE void intercom_process_tx_data_event(Intercom* instance) {
    if(instance->state == IntercomStateIdle) {
        instance->state = IntercomStateWaitingConfirmation;
        intercom_send_data_frame(instance);
    } else {
        // Unexpected state - should not ever get here
        furi_crash();
    }
}

static FURI_ALWAYS_INLINE void intercom_process_rx_frame_event(Intercom* instance) {
    INTERCOM_LOG_D("Frame received");

    const IntercomFrame* rx_frame = &instance->rx_frame;
    const bool frame_is_valid = intercom_frame_is_valid(rx_frame);

    if(instance->state == IntercomStateIdle) {
        INTERCOM_LOG_D("D-frame expected");

        if(!frame_is_valid) {
            // Frame is invalid, a D-frame was expected
            FURI_LOG_E(TAG, "[Idle] Invalid frame");
            intercom_send_service_frame(instance, IntercomFrameErrorFormat, INTERCOM_D_FRAME_SIZE);

        } else if(rx_frame->header.type == IntercomFrameTypeData) {
            INTERCOM_LOG_D("D-frame received");

            const IntercomFramePayload* payload = &rx_frame->d.payload;

            if(instance->rx_callback) {
                instance->rx_callback(payload->data, payload->size, instance->callback_context);
            }

            // Confirm the incoming D-frame and start listening for the next one
            intercom_send_service_frame(instance, IntercomFrameErrorNone, INTERCOM_D_FRAME_SIZE);

        } else {
            // Frame valid, but unexpected
            // TODO: send the frame after a random delay
            FURI_LOG_E(TAG, "[Idle] Unexpected frame");
            intercom_send_service_frame(
                instance, IntercomFrameErrorWrongType, INTERCOM_D_FRAME_SIZE);
        }

    } else if(instance->state == IntercomStateWaitingConfirmation) {
        INTERCOM_LOG_D("S-frame expected");
        furi_event_loop_timer_stop(instance->response_timer);

        if(!frame_is_valid) {
            // Frame is invalid, an S-frame was expected
            FURI_LOG_E(TAG, "[Waiting] Invalid frame");
            intercom_send_service_frame(instance, IntercomFrameErrorFormat, INTERCOM_S_FRAME_SIZE);

        } else if(rx_frame->header.type == IntercomFrameTypeService) {
            INTERCOM_LOG_D("S-frame received");

            const IntercomFrameError error = rx_frame->header.error;
            if(error == IntercomFrameErrorNone) {
                instance->state = IntercomStateIdle;
                furi_hal_serial_dma_rx_start(
                    instance->serial, (uint8_t*)&instance->rx_frame, INTERCOM_D_FRAME_SIZE);
                furi_semaphore_release(instance->tx_semaphore);

            } else {
                // Frame is valid and of proper type, but reports and error - resend current tx frame
                FURI_LOG_E(TAG, "Resending frame");
                intercom_send_data_frame(instance);
            }

        } else {
            // Frame valid, but unexpected
            // TODO: send the frame after a random delay
            FURI_LOG_E(TAG, "[Waiting] Unexpected frame");
            intercom_send_service_frame(
                instance, IntercomFrameErrorWrongType, INTERCOM_S_FRAME_SIZE);
        }
    }
}

static FURI_ALWAYS_INLINE void intercom_process_tx_frame_event(Intercom* instance) {
    if(instance->state == IntercomStateWaitingConfirmation) {
        furi_event_loop_timer_start(instance->response_timer, INTERCOM_RESPONSE_TIMEOUT_MS);
    }

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

static void intercom_response_timer_callback(void* context) {
    Intercom* instance = context;

    FURI_LOG_E(TAG, "Response timeout");
    // Resend the frame
    intercom_send_data_frame(instance);
}

static Intercom* intercom_alloc(void) {
    Intercom* instance = malloc(sizeof(Intercom));

    instance->event_loop = furi_event_loop_alloc();

    instance->tx_semaphore = furi_semaphore_alloc(1, 1);
    instance->response_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        intercom_response_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);

    instance->serial = furi_hal_serial_control_acquire(INTERCOM_SERIAL);
    furi_hal_serial_init(instance->serial, INTERCOM_BAUD_RATE);
    furi_hal_serial_set_hw_flow_control(instance->serial, FuriHalSerialHwFlowControlRtsCts);
    furi_hal_serial_set_callback(
        instance->serial, intercom_serial_tx_callback, intercom_serial_rx_callback, instance);
    // Start listening for D-frames right away
    furi_hal_serial_dma_rx_start(
        instance->serial, (uint8_t*)&instance->rx_frame, INTERCOM_D_FRAME_SIZE);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, intercom_custom_event_callback, instance);

    furi_record_create(RECORD_INTERCOM, instance);
    return instance;
}

size_t intercom_tx(Intercom* instance, const void* data, size_t data_size, uint32_t timeout) {
    furi_check(instance);
    furi_check(data);
    furi_check(data_size > 0);

    size_t sent_data_size = 0;

    const uint32_t start_time = furi_get_tick();
    uint32_t remaining_time = timeout;

    while(furi_semaphore_acquire(instance->tx_semaphore, remaining_time) == FuriStatusOk) {
        const size_t payload_size = MIN(data_size - sent_data_size, INTERCOM_D_FRAME_DATA_SIZE);

        memcpy(instance->tx_frame.d.payload.data, data + sent_data_size, payload_size);
        instance->tx_frame.d.payload.size = payload_size;

        INTERCOM_LOG_D("TX payload size: %zu byte(s)", payload_size);
        furi_event_loop_set_custom_event(instance->event_loop, IntercomEventData);

        sent_data_size += payload_size;
        if(sent_data_size == data_size) break;

        const uint32_t elapsed_time = furi_get_tick() - start_time;
        if(elapsed_time >= remaining_time) break;

        remaining_time -= elapsed_time;
    }

    return sent_data_size;
}

void intercom_set_rx_callback(Intercom* instance, IntercomRxCallback callback, void* context) {
    furi_check(instance);
    furi_check(callback);
    furi_check(instance->rx_callback == NULL);

    instance->callback_context = context;
    instance->rx_callback = callback;
}

int32_t intercom_srv(void* arg) {
    UNUSED(arg);

    Intercom* instance = intercom_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
