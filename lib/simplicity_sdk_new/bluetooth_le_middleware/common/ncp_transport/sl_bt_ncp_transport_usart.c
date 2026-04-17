/***************************************************************************//**
 * @file
 * @brief Bluetooth NCP Transport Layer over USART
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "sl_status.h"
#include "sl_common.h"
#include "sl_core.h"
#include "sl_clock_manager.h"
#include "sl_uartdrv_instances.h"
#include "app_rta.h"
#include "sl_bt_ncp_transport.h"
#include "sli_bt_ncp_transport.h"
#include "sl_bt_ncp_transport_usart_config.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#include "app_assert.h"
#endif // SL_CATALOG_APP_ASSERT_PRESENT

#define RX_MSG_SIZE_HINT (SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE / 2)
#define RTA_QUEUE_SIZE 4
#define ALIGNED_ADDRESS(base, offset) \
  (base + ((offset + sizeof(void *) - 1) & (~0x3)))

// Store UARTDRV handle
static UARTDRV_Handle_t uartdrv_handle = NULL;
static IRQn_Type irq_number;

// Application Runtime Adaptor context
static app_rta_context_t ctx = APP_RTA_INVALID_CONTEXT;

// store the returned values of TX callback
typedef struct {
  UARTDRV_Handle_t handle;
  uint32_t         timeout;
  sl_status_t      status;
  bool             finished;
} tx_cb_sig;

typedef enum {
  EVENT_RX_REQUEST,
  EVENT_RX_FINISHED,
  EVENT_TX_FINISHED
} event_type_t;

typedef struct {
  sl_status_t status;
  size_t      len;
  void        *data;
} evt_rx_finished_t;

typedef struct {
  sl_status_t status;
} evt_tx_finished_t;

typedef struct {
  event_type_t        event_type;
  union {
    evt_rx_finished_t evt_rx_finished;
    evt_tx_finished_t evt_tx_finished;
  } data;
} event_t;

// These variables are used in both interrupt and application level contexts:
static volatile tx_cb_sig tx_cb_signal = { 0 };

// UART receive and transmit buffers
static __ALIGNED(4) uint8_t rx_buf[SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE] = { 0 };
static __ALIGNED(4) uint8_t tx_buf[SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE] = { 0 };

// Instead of relying on a single, constant buffer address, we will rather use
// a "circular buffer-like" technique. This approach is cheaper than the general
// implementation, however, because it uses only a single circular pointer; it
// does not involve head and tail pointers or other complexities.
static uint8_t *dma_rx_ptr = rx_buf;
static uint8_t *dma_tx_ptr = tx_buf;

// Internal UART receive and transmit callback
static void transmit_cb(UARTDRV_Handle_t handle,
                        Ecode_t transferStatus,
                        uint8_t *data,
                        UARTDRV_Count_t transferCount);
static void receive_cb(UARTDRV_Handle_t handle,
                       Ecode_t transferStatus,
                       uint8_t *data,
                       UARTDRV_Count_t transferCount);
// Helper functions to ensure reception
static Ecode_t cancel_receive(UARTDRV_Handle_t handle);
static Ecode_t dequeue_buffer(UARTDRV_Buffer_FifoQueue_t *queue,
                              UARTDRV_Buffer_t **buffer);
static IRQn_Type irq_number_from_handle(UARTDRV_Handle_t handle);
static Ecode_t uart_receive_start(UARTDRV_Handle_t handle);
void sli_bt_ncp_transport_usart_isr(void);
static void on_runtime_error(app_rta_error_t error,
                             sl_status_t     result);
static void send_read_request(void);

// -----------------------------------------------------------------------------
// Public functions (API implementation)

/*****************************************************************************
* Bluetooth NCP Transport Init.
*****************************************************************************/
void sli_bt_ncp_transport_init(void)
{
  // Create context
  sl_status_t sc;
  app_rta_config_t config = {
    .requirement.runtime = true,
    .requirement.guard   = true,
    .requirement.signal  = false,
    .requirement.queue   = true,
    .step                = sli_bt_ncp_transport_step,
    .priority            = SL_BT_NCP_TRANSPORT_USART_CONFIG_TASK_PRIO,
    .stack_size          = SL_BT_NCP_TRANSPORT_USART_CONFIG_TASK_STACK,
    .error               = on_runtime_error,
    .wait_for_guard      = SL_BT_NCP_TRANSPORT_USART_CONFIG_WAIT_FOR_GUARD,
    .queue_size          = RTA_QUEUE_SIZE,
    .queue_element_size  = sizeof(event_t)
  };
  sc = app_rta_create_context(&config, &ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
  }
}

void sli_bt_ncp_transport_rta_ready(void)
{
  sl_status_t sc;
  uint32_t frequency;

  // Get the default UARTDRV handle
  uartdrv_handle = sl_uartdrv_get_default();
  irq_number = irq_number_from_handle(uartdrv_handle);
  if (irq_number < (IRQn_Type)(0)) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_COM_HANDLE,
                                 SL_STATUS_INVALID_HANDLE);
  }

  // Calculate the maximum amount of time to wait for UART TX buffer to empty
  sc = sl_clock_manager_get_clock_branch_frequency(SL_CLOCK_BRANCH_HCLK, &frequency);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_CLOCK, sc);
  }
}

/******************************************************************************
 * UART transmit function
 *
 * Transmits len bytes of data through the UART interface using DMA.
 *
 * @param[out] len Message length
 * @param[out] data Message data
 *****************************************************************************/
void sl_bt_ncp_transport_transmit(uint32_t len, const uint8_t *data)
{
  sl_status_t sc;
  Ecode_t ec;

  // Acquire guard
  sc = app_rta_acquire(ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }

  if (ALIGNED_ADDRESS(dma_tx_ptr, len) >= &tx_buf[SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE]) {
    // previous DMA may be in progress on sl_bt_ncp_transport_transmit() being
    // called so with a buffer size optimized well for a given task the DMA can
    // keep going with previous data before those being overwritten in memory by
    // the caller
    dma_tx_ptr = tx_buf;
  }
  // Make a copy of the data to be sent to guarantee its integrity until
  // transmission completes
  if (len > SL_BT_NCP_TRANSPORT_CONFIG_TX_BUF_SIZE) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_TX_OVERFLOW,
                                 SL_STATUS_WOULD_OVERFLOW);
    // Release guard
    app_rta_release(ctx);
    return;
  }
  memcpy((void *)dma_tx_ptr, (const void *)data, (size_t)len);

  // Transmit data using a non-blocking transmit function
  ec = UARTDRV_Transmit(uartdrv_handle,
                        dma_tx_ptr,
                        len,
                        transmit_cb);

  if (ec == ECODE_EMDRV_UARTDRV_OK) {
    // Only update the moving pointer on successful transmit requests.
    dma_tx_ptr = ALIGNED_ADDRESS(dma_tx_ptr, len);
  }
  (void)app_rta_proceed(ctx);
  // Release guard
  (void)app_rta_release(ctx);
}

/**************************************************************************//**
 * UART receive function
 *
 * Starts reception on UART interface using DMA.
 *
 * @note The problem with standard UART reception is that it needs the exact
 * amount of bytes to read. This implementation starts a timer to measure idle
 * time on the bus and based on that puts the received message into the buffer.
 *****************************************************************************/
void sl_bt_ncp_transport_receive(void)
{
  sl_status_t sc;

  // Acquire guard
  sc = app_rta_acquire(ctx);
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }

  // Clear pending RX interrupt flag in NVIC
  NVIC_ClearPendingIRQ(irq_number);
  NVIC_EnableIRQ(irq_number);
  // Setup RX timeout to 255 bit-time
  uartdrv_handle->peripheral.uart->TIMECMP1 = \
    (USART_TIMECMP1_TSTOP_RXACT
     | USART_TIMECMP1_TSTART_RXEOF
     | USART_TIMECMP1_RESTARTEN
     | (0xff << _USART_TIMECMP1_TCMPVAL_SHIFT));
  // Clear any USART interrupt flags
  USART_IntClear(uartdrv_handle->peripheral.uart, _USART_IF_MASK);

  CORE_ATOMIC_SECTION(
    Ecode_t ec = uart_receive_start(uartdrv_handle);
    // try deferred start on error
    if ((ec != ECODE_EMDRV_UARTDRV_OK)) {
    send_read_request();
  }
    )

  // Release guard
    (void) app_rta_release(ctx);
}

/**************************************************************************//**
 * UART transmit completed callback
 *
 * Called after UART transmit is finished.
 *
 * @param[in] status Status of the transmission
 *
 * @note Weak implementation
 *  Called after DMA is complete, the UART transmission might be still ongoing
 *****************************************************************************/
SL_WEAK void sl_bt_ncp_transport_on_transmit(sl_status_t status)
{
  (void)status;
}

/**************************************************************************//**
 * UART receive completed callback
 *
 * Called after UART receive is finished.
 *
 * @param[in] status Status of the reception
 * @param[in] len Received message length
 * @param[in] data Data received
 *
 * @note Weak implementation
 *****************************************************************************/
SL_WEAK void sl_bt_ncp_transport_on_receive(sl_status_t status,
                                            uint32_t len,
                                            uint8_t *data)
{
  (void)status;
  (void)data;
  (void)len;
}

/*****************************************************************************
* Step function
*****************************************************************************/
void sli_bt_ncp_transport_step(void)
{
  CORE_DECLARE_IRQ_STATE;
  event_t evt;
  sl_status_t sc;
  size_t size;
  sc = app_rta_queue_read_and_acquire(ctx, (uint8_t *)&evt, &size);
  if (sc == SL_STATUS_EMPTY) {
    // No event available
    return;
  } else if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    return;
  }

  CORE_ENTER_ATOMIC();
  switch (evt.event_type) {
    case EVENT_TX_FINISHED:
      // Transfer finished
      sc = evt.data.evt_tx_finished.status;
      // Clear TX buffer
      tx_cb_signal.timeout = 0;
      tx_cb_signal.finished = false;
      // Exit critical section before calling user code is mandatory.
      CORE_EXIT_ATOMIC();
      (void)app_rta_release(ctx);
      // Call public callback API
      sl_bt_ncp_transport_on_transmit(sc);
      break;
    case EVENT_RX_REQUEST:
      // After each completed DMA transfer, whether it is aborted by a TCMP1 RX
      // timeout or whether it is simply a successful block receive, the receiver
      // must be restarted.
      if (UARTDRV_GetReceiveDepth(uartdrv_handle) == 0) {
        uart_receive_start(uartdrv_handle);
      }
      CORE_EXIT_ATOMIC();
      (void)app_rta_release(ctx);
      break;
    case EVENT_RX_FINISHED:
      CORE_EXIT_ATOMIC();
      (void)app_rta_release(ctx);
      // Call public callback API
      sl_bt_ncp_transport_on_receive(evt.data.evt_rx_finished.status,
                                     evt.data.evt_rx_finished.len,
                                     evt.data.evt_rx_finished.data);

      break;
    default:
      CORE_EXIT_ATOMIC();
      (void)app_rta_release(ctx);
      break;
  }
}

// -----------------------------------------------------------------------------
// Private functions

/******************************************************************************
 * UART interrupt handler
 *
 * Called when the set timer for tx idle states finished.
 *
 * @note automatically restarts the timer and starts a new reception.
 * @note TODO: this should be provided by UARTDRV
 *****************************************************************************/
void sli_bt_ncp_transport_usart_isr(void)
{
  // RX timeout, stop transfer and handle what we got in buffer
  if (uartdrv_handle->peripheral.uart->IF & USART_IF_TCMP1) {
    CORE_DECLARE_IRQ_STATE;

    // Assert nRTS
    (void)UARTDRV_FlowControlSet(uartdrv_handle, uartdrvFlowControlOff);

    // Atomic section ensures that other ISRs with lower or equal priorities won't disrupt these operations.
    CORE_ENTER_ATOMIC();
    // Mask further TCMP1 interrupts (until receive will be restarted)
    USART_IntDisable(uartdrv_handle->peripheral.uart, USART_IF_TCMP1);

    // Clear current timer compare interrupt flag
    USART_IntClear(uartdrv_handle->peripheral.uart,
                   USART_IF_TCMP1);

    // Raise to critical section to make the "blind spot" on RX (for the START edge) as short as possible.
    CORE_CRITICAL_SECTION(
      // TCMP1 needs to be stopped in order to be able to save power once we received a whole BGAPI message.
      uartdrv_handle->peripheral.uart->TIMECMP1 &= ~_USART_TIMECMP1_TSTART_MASK;
      // Restart TCMP1 as the RX timeout timer and let it run "free" as soon as a new START bit is detected.
      uartdrv_handle->peripheral.uart->TIMECMP1 |= USART_TIMECMP1_TSTART_RXEOF
                                                   | USART_TIMECMP1_RESTARTEN; )
    CORE_EXIT_ATOMIC();
    // Cancel previous block receive operation
    (void) cancel_receive(uartdrv_handle);
  }
  if (uartdrv_handle->peripheral.uart->IF & USART_IF_RXOF) {
    // Assert nRTS
    (void)UARTDRV_FlowControlSet(uartdrv_handle, uartdrvFlowControlOff);
    // The RX overflow error must be handled in order to keep bt_ncp_transport
    // working even with the flow control method set to "None" (disabled).
    (void)UARTDRV_Abort(uartdrv_handle, uartdrvAbortReceive); // This call may do some RX state cleanup

    USART_IntClear(uartdrv_handle->peripheral.uart, USART_IF_RXOF);
  }
}

void sli_bt_ncp_transport_usart_cancel_receive(void)
{
  (void) cancel_receive(uartdrv_handle);
}

/*****************************************************************************
* Error handler weak implementation
*****************************************************************************/
SL_WEAK void sl_bt_ncp_transport_on_error(sl_bt_ncp_transport_error_t error,
                                          sl_status_t status)
{
  (void)error;
  (void)status;
  #ifdef SL_CATALOG_APP_ASSERT_PRESENT
  app_assert_status_f(status,
                      "blutetooth_ncp_transport: Error %u occurred.",
                      error);
  #endif // SL_CATALOG_APP_ASSERT_PRESENT
}

/******************************************************************************
 * Internal UART transmit completed callback
 *
 * Called after UART transmit is finished.
 *
 * @param[in] handle UART driver handle
 * @param[in] transferStatus Status of the transfer
 * @param[in] data Transmitted data
 * @param[in] transferCount Number of sent bytes
 *
 * @note Calls public sl_bt_ncp_transport_on_transmit
 *  Called after DMA is complete, the UART transmission might be still ongoing
 *****************************************************************************/
static void transmit_cb(UARTDRV_Handle_t handle,
                        Ecode_t transferStatus,
                        uint8_t *data,
                        UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)data;
  (void)transferCount;

  sl_status_t sc;

  event_t     evt;
  evt.event_type = EVENT_TX_FINISHED;
  evt.data.evt_tx_finished.status
    = (ECODE_EMDRV_UARTDRV_OK == transferStatus) ? SL_STATUS_OK : SL_STATUS_FAIL;

  // Emit event
  sc = app_rta_queue_push(ctx, (uint8_t *)&evt, sizeof(evt));
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
  }
}

/******************************************************************************
 * Internal UART receive completed callback
 *
 * Called after UART receive is finished.
 *
 * @param[in] handle UART driver handle
 * @param[in] transferStatus Status of the transfer
 * @param[in] data Received data
 * @param[in] transferCount Number of received bytes
 *
 * @note Calls public sl_bt_ncp_transport_on_receive
 *****************************************************************************/
static void receive_cb(UARTDRV_Handle_t handle,
                       Ecode_t transferStatus,
                       uint8_t *data,
                       UARTDRV_Count_t transferCount)
{
  (void)handle;

  sl_status_t sc;
  event_t evt;

  evt.event_type = EVENT_RX_FINISHED;
  evt.data.evt_rx_finished.status = SL_STATUS_OK;
  evt.data.evt_rx_finished.data   = NULL;
  evt.data.evt_rx_finished.len    = 0;

  // Check status
  if (transferStatus != ECODE_EMDRV_UARTDRV_ABORTED
      && transferStatus != ECODE_EMDRV_UARTDRV_OK) {
    evt.data.evt_rx_finished.status = SL_STATUS_FAIL;
  }

  if (transferCount != 0) {
    evt.data.evt_rx_finished.len  = transferCount;
    evt.data.evt_rx_finished.data = data;
    // Emit event
    sc = app_rta_queue_push(ctx, (uint8_t *)&evt, sizeof(evt));
    if (sc != SL_STATUS_OK) {
      sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
    }
  }
  send_read_request();
}

/******************************************************************************
 * Cancel previous block receive operation.
 *
 * @param[in] handle UART driver handle
 *****************************************************************************/
static Ecode_t cancel_receive(UARTDRV_Handle_t handle)
{
  UARTDRV_Buffer_t *rxBuffer;
  Ecode_t status;
  CORE_DECLARE_IRQ_STATE;

  if (handle == NULL) {
    return ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE;
  }

  CORE_ENTER_ATOMIC();
  if (handle->rxQueue->used == 0) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_UARTDRV_IDLE;
  }

  // -------------------------------
  // Stop the current DMA transfer
  (void)DMADRV_StopTransfer(handle->rxDmaCh);
  handle->rxDmaActive = false;
  // Update the transfer status of the active transfer
  status = dequeue_buffer(handle->rxQueue, &rxBuffer);
  // If aborting was in progress when DMA completed, the ISR could be deferred
  // until after the critical section. In this case, the buffers no longer
  // exist, even though the DMA complete callback was called.
  if (status == ECODE_EMDRV_UARTDRV_QUEUE_EMPTY) {
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_UARTDRV_QUEUE_EMPTY;
  }
  status = DMADRV_TransferRemainingCount(handle->rxDmaCh,
                                         (int *)&rxBuffer->itemsRemaining);
  rxBuffer->transferStatus = ECODE_EMDRV_UARTDRV_ABORTED;

  // Call the callback
  if (status == ECODE_EMDRV_UARTDRV_OK && rxBuffer->callback != NULL) {
    rxBuffer->callback(handle,
                       rxBuffer->transferStatus,
                       rxBuffer->data,
                       rxBuffer->transferCount - rxBuffer->itemsRemaining);
  }

  // -------------------------------
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_UARTDRV_OK;
}

/******************************************************************************
 * Dequeues buffer
 *
 * Moves through the buffer.
 *
 * @param[in] queue Input buffer
 * @param[out] buffer Output buffer
 *****************************************************************************/
static Ecode_t dequeue_buffer(UARTDRV_Buffer_FifoQueue_t *queue,
                              UARTDRV_Buffer_t **buffer)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  if (queue->used == 0) {
    *buffer = NULL;
    CORE_EXIT_ATOMIC();
    return ECODE_EMDRV_UARTDRV_QUEUE_EMPTY;
  }
  *buffer = &queue->fifo[queue->tail];
  queue->tail = (queue->tail + 1) % queue->size;
  queue->used--;
  CORE_EXIT_ATOMIC();

  return ECODE_EMDRV_UARTDRV_OK;
}

/******************************************************************************
 * Get NVIC IRQ number from UARTDRV handle
 *
 * @param[in] handle UART driver handle
 * @return Interrupt number
 *****************************************************************************/
static IRQn_Type irq_number_from_handle(UARTDRV_Handle_t handle)
{
  IRQn_Type irq_number = (IRQn_Type)(-1);

  switch ((uint32_t)handle->peripheral.uart) {
#if defined(USART0_BASE)
    case USART0_BASE:
      irq_number = USART0_RX_IRQn;
      break;
#endif
#if defined(USART1_BASE)
    case USART1_BASE:
      irq_number = USART1_RX_IRQn;
      break;
#endif
#if defined(USART2_BASE)
    case USART2_BASE:
      irq_number = USART2_RX_IRQn;
      break;
#endif
#if defined(USART3_BASE)
    case USART3_BASE:
      irq_number = USART3_RX_IRQn;
      break;
#endif
    default:
      sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_COM_HANDLE,
                                   SL_STATUS_INVALID_HANDLE);
      break;
  }

  return irq_number;
}

/******************************************************************************
 * Start / resume UARTDRV receiving
 *
 * @param[in] handle UART driver handle
 * @return UARTDRV reported result of the operation
 *****************************************************************************/
static Ecode_t uart_receive_start(UARTDRV_Handle_t handle)
{
  CORE_DECLARE_IRQ_STATE;
  Ecode_t ec;
  uint16_t len = RX_MSG_SIZE_HINT;

  if (ALIGNED_ADDRESS(dma_rx_ptr, len) >= &rx_buf[SL_BT_NCP_TRANSPORT_CONFIG_RX_BUF_SIZE]) {
    // previous DMA may be in progress on uart_receive_start() being called
    // so with a buffer size optimized well for a given task the DMA can keep
    // going with previous data before those being overwritten in memory by the
    // DMA
    dma_rx_ptr = rx_buf;
  }

  CORE_ENTER_ATOMIC();
  ec = UARTDRV_Receive(handle, dma_rx_ptr, len, receive_cb);
  // The receive request must be cleared (or left set, depending on the result)
  // within the critical section - otherwise receiving may stop unexpectedly.
  if (ec != ECODE_EMDRV_UARTDRV_OK) {
    send_read_request();
  }
  CORE_EXIT_ATOMIC();

  if (ec == ECODE_EMDRV_UARTDRV_OK) {
    USART_IntClear(uartdrv_handle->peripheral.uart,
                   USART_IF_TCMP1);
    USART_IntEnable(uartdrv_handle->peripheral.uart,
                    (USART_IF_TCMP1 | USART_IF_RXOF));
    // De-assert nRTS or send XON
    (void)UARTDRV_FlowControlSet(uartdrv_handle, uartdrvFlowControlOn);
    dma_rx_ptr = ALIGNED_ADDRESS(dma_rx_ptr, len);
  }

  return ec;
}

// Error callback
static void on_runtime_error(app_rta_error_t error,
                             sl_status_t     result)
{
  (void)error;
  sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, result);
}

static void send_read_request(void)
{
  sl_status_t sc;
  event_t evt;
  evt.event_type = EVENT_RX_REQUEST;
  sc = app_rta_queue_push(ctx, (uint8_t *)&evt, sizeof(evt));
  if (sc != SL_STATUS_OK) {
    sl_bt_ncp_transport_on_error(SL_BT_NCP_TRANSPORT_ERROR_RUNTIME, sc);
  }
}
