/**
 * @file furi_hal_serial.h
 *
 * Serial HAL API
 */
#pragma once

#include <furi_hal_serial_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Serial hardware flow control modes */
typedef enum {
    FuriHalSerialHwFlowControlNone, /**< Do not use any hardware flow control */
    FuriHalSerialHwFlowControlRts, /**< Use the Request to send (RTS) signal only */
    FuriHalSerialHwFlowControlCts, /**< Use the Clear to send (CTS) signal only */
    FuriHalSerialHwFlowControlRtsCts, /**< Use both RTS and CTS signals */
} FuriHalSerialHwFlowControl;

/** Serial RX events */
typedef enum {
    FuriHalSerialRxEventData = (1 << 0), /**< Data: new data available */
    FuriHalSerialRxEventIdle = (1 << 1), /**< Idle: bus idle detected */
    FuriHalSerialRxEventFrameError = (1 << 2), /**< Framing Error: incorrect frame detected */
    FuriHalSerialRxEventNoiseError = (1 << 3), /**< Noise Error: noise on the line detected */
    FuriHalSerialRxEventParityError = (1 << 4), /**< Parity Error: incorrect parity detected */
    FuriHalSerialRxEventOverrunError = (1 << 5), /**< Overrun Error: no space for received data */
} FuriHalSerialRxEvent;

/** Serial TX events */
typedef enum {
    FuriHalSerialTxEventComplete = (1 << 0), /**< Transmission complete */
} FuriHalSerialTxEvent;

typedef void (*FuriHalSerialRxCallback)(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context);

typedef void (*FuriHalSerialTxCallback)(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxEvent event,
    void* context);

/**
 * Initialize the serial interface.
 *
 * @param handle Pointer to the serial handle.
 * @param baud_rate Baud rate.
 *      @arg min: 10UL
 *      @arg max: 20000000UL
 */
void furi_hal_serial_init(FuriHalSerialHandle* handle, uint32_t baud_rate);

/**
 * Deinitialize the serial interface.
 *
 * @param handle Pointer to the serial handle.
 */
void furi_hal_serial_deinit(FuriHalSerialHandle* handle);

/**
 * Suspend the serial interface.
 * @param handle Pointer to the serial handle.
 */
void furi_hal_serial_suspend(FuriHalSerialHandle* handle);

/**
 * Resume the serial interface.
 * @param handle Pointer to the serial handle.
 */
void furi_hal_serial_resume(FuriHalSerialHandle* handle);

bool furi_hal_serial_is_baud_rate_supported(FuriHalSerialHandle* handle, uint32_t baud);

/**
 * Set the baud rate for the serial interface.
 *
 * @param handle Pointer to the serial handle.
 * @param baud_rate Baud rate.
 *      @arg min: 10UL
 *      @arg max: 20000000UL
 */
void furi_hal_serial_set_baudrate(FuriHalSerialHandle* handle, uint32_t baud_rate);

/**
 * Set the hardware flow control mode for the serial interface.
 *
 * @param handle Pointer to the serial handle.
 * @param flow_control Flow control mode.
 */
void furi_hal_serial_set_hw_flow_control(
    FuriHalSerialHandle* handle,
    FuriHalSerialHwFlowControl flow_control);

/**
 * Set the callback functions for the serial interface.
 *
 * @param handle Pointer to the serial handle.
 * @param tx_callback Pointer to the transmit callback function.
 * @param rx_callback Pointer to the receive callback function.
 * @param context Pointer to the context object.
 */
void furi_hal_serial_set_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxCallback tx_callback,
    FuriHalSerialRxCallback rx_callback,
    void* context);

/* Blocking API */

/**
 * Transmit data over the serial interface.
 *
 * @param handle Pointer to the serial handle.
 * @param buffer Pointer to the data buffer.
 * @param buffer_size Size of the data buffer.
 */
void furi_hal_serial_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size);

/**
 * Wait for the transmission to complete.
 * @param handle Pointer to the serial handle.
 */
void furi_hal_serial_tx_wait_complete(FuriHalSerialHandle* handle);

/* Interrupt-based asynchronous API */

void furi_hal_serial_async_rx_start(FuriHalSerialHandle* handle, bool report_errors);

void furi_hal_serial_async_rx_stop(FuriHalSerialHandle* handle);

size_t furi_hal_serial_async_rx(FuriHalSerialHandle* handle, uint8_t* buffer, size_t buffer_size);

/* DMA-based asynchronous API */

/**
 * Transmit data over the serial interface using DMA.
 *
 * @param handle Pointer to the serial handle.
 * @param buffer Pointer to the data buffer.
 * @param buffer_size Size of the data buffer.
 */
void furi_hal_serial_dma_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size);

void furi_hal_serial_dma_rx_start(FuriHalSerialHandle* handle, uint8_t* buffer, size_t buffer_size);

void furi_hal_serial_dma_rx_stop(FuriHalSerialHandle* handle);
