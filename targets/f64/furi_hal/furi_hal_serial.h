/**
 * @file furi_hal_serial.h
 * 
 * Serial HAL API
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <furi_hal_serial_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Serial hardware flow control modes */
typedef enum {
    FuriHalSerialHwFlowControlNone, /**< Do not use any hardware flow control */
    FuriHalSerialHwFlowControlRtsCts, /**< Use both RTS and CTS signals */
} FuriHalSerialHwFlowControl;

/** Serial RX events */
typedef enum {
    FuriHalSerialRxEventData = (1 << 0), /**< Data: new data available */
    FuriHalSerialRxEventIdle = (1 << 1), /**< Idle: bus idle detected */
    FuriHalSerialRxEventBreak = (1 << 2), /**< Break: break condition detected */
    FuriHalSerialRxEventFrameError = (1 << 3), /**< Framing Error: incorrect frame detected */
    FuriHalSerialRxEventNoiseError = (1 << 4), /**< Noise Error: noise on the line detected */
    FuriHalSerialRxEventOverrunError = (1 << 5), /**< Overrun Error: no space for received data */
} FuriHalSerialRxEvent;

/** Serial TX events */
typedef enum {
    FuriHalSerialTxEventComplete = (1 << 0), /**< Transmission complete */
} FuriHalSerialTxEvent;

typedef void (*FuriHalSerialTxCallback)(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxEvent event,
    void* context);

typedef void (*FuriHalSerialRxCallback)(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context);

void furi_hal_serial_init(FuriHalSerialHandle* handle, uint32_t baud);

void furi_hal_serial_deinit(FuriHalSerialHandle* handle);

void furi_hal_serial_suspend(FuriHalSerialHandle* handle);

void furi_hal_serial_resume(FuriHalSerialHandle* handle);

bool furi_hal_serial_is_baud_rate_supported(FuriHalSerialHandle* handle, uint32_t baud);

void furi_hal_serial_set_br(FuriHalSerialHandle* handle, uint32_t baud);

/**
 * Set the hardware flow control mode for the serial interface.
 *
 * @param handle Pointer to the serial handle.
 * @param flow_control Flow control mode.
 */
void furi_hal_serial_set_hw_flow_control(
    FuriHalSerialHandle* handle,
    FuriHalSerialHwFlowControl flow_control);

void furi_hal_serial_set_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialTxCallback tx_callback,
    FuriHalSerialRxCallback rx_callback,
    void* context);

/* Blocking API */

void furi_hal_serial_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size);

void furi_hal_serial_tx_wait_complete(FuriHalSerialHandle* handle);

/* Interrupt-based asynchronous API */

void furi_hal_serial_async_rx_start(FuriHalSerialHandle* handle, bool report_errors);

void furi_hal_serial_async_rx_stop(FuriHalSerialHandle* handle);

bool furi_hal_serial_async_rx_available(FuriHalSerialHandle* handle);

uint8_t furi_hal_serial_async_rx(FuriHalSerialHandle* handle);

/* DMA-based asynchronous API */

void furi_hal_serial_dma_tx(FuriHalSerialHandle* handle, const uint8_t* buffer, size_t buffer_size);

void furi_hal_serial_dma_rx_start(FuriHalSerialHandle* handle, uint8_t* buffer, size_t buffer_size);

void furi_hal_serial_dma_rx_stop(FuriHalSerialHandle* handle);

#ifdef __cplusplus
}
#endif
