/***************************************************************************//**
 * @file
 * @brief IO Stream UART Component.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
// Define module name for Power Manager debug feature
#define CURRENT_MODULE_NAME    "IOSTREAM_UART"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_status.h"
#include "sl_iostream.h"
#include "sli_iostream.h"
#include "sl_iostream_uart.h"
#include "sli_iostream_uart.h"
#include "sl_atomic.h"
#include "sl_slist.h"
#include "sl_string.h"
#include "em_device.h"

#if (defined(SL_CATALOG_KERNEL_PRESENT))
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#endif

#if (defined(SL_CATALOG_POWER_MANAGER_PRESENT))
#include "sl_power_manager.h"
#endif

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dmadrv.h"
#include "em_device.h"
#include "sl_core.h"
#include "sl_assert.h"

#if !defined(DMA_PRESENT) && !defined(LDMA_PRESENT)
#error Missing (L)DMA peripheral
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#define MAX_RX_FIFO_DEPTH 16  ///< Used to limit iterations in RX DMA IRQ handler
#define RX_DATA_AVAILABLE_FLAG  1

/*******************************************************************************
 **************************** LOCAL VARIABLES **********************************
 ******************************************************************************/
// Byte used by the LDMA in the new data detection mechanism.
static uint8_t null_byte;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t uart_deinit(void *stream);

static sl_status_t uart_write(void *context,
                              const void *buffer,
                              size_t buffer_length);

static sl_status_t uart_write_async(void *context,
                                    sli_iostream_write_async_op_t *async_op);

static sl_status_t uart_read(void *context,
                             void *buffer,
                             size_t buffer_length,
                             size_t *bytes_read);

static void set_auto_cr_lf(void *context,
                           bool on);

static bool get_auto_cr_lf(void *context);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#if !defined(SL_CATALOG_KERNEL_PRESENT)
static bool wakeup_from_rx(const sl_iostream_uart_context_t *uart_context);

static sl_power_manager_on_isr_exit_t sleep_on_isr_exit(void *context);
#endif

static void set_rx_energy_mode_restriction(void *context,
                                           bool on);

static bool get_rx_energy_mode_restriction(void *context);
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
static void set_read_block(void *context,
                           bool on);

static bool get_read_block(void *context);
#endif

static void scan_for_ctrl_char(sl_iostream_uart_context_t * uart_context);

static sl_status_t nolock_uart_write(void *context,
                                     const void *buffer,
                                     size_t buffer_length);

static inline bool __rx_buffer_full(const sl_iostream_uart_context_t *uart_context);

static inline bool rx_buffer_empty(const sl_iostream_uart_context_t *uart_context);

static inline bool __rx_buffer_empty(const sl_iostream_uart_context_t *uart_context);

static inline uint8_t* __get_write_ptr(const sl_iostream_uart_context_t * uart_context);

static inline size_t __get_bytes_available(const sl_iostream_uart_context_t *uart_context);

static inline void set_new_data_detect(sl_iostream_uart_context_t *uart_context);

static bool __uart_async_rx_dma_callback(unsigned int channel, unsigned int sequenceNo,
                                         void *userParam);

static void update_ring_buffer(sl_iostream_uart_context_t *uart_context, size_t read_size);

static size_t read_rx_buffer(sl_iostream_uart_context_t * uart_context,
                             uint8_t * buffer,
                             size_t buffer_len);

static void __uart_async_start_write(sli_iostream_write_async_op_t *async_op);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * UART Stream init
 ******************************************************************************/
sl_status_t sli_iostream_uart_context_init(sl_iostream_uart_t *uart,
                                           sl_iostream_uart_context_t *context,
                                           sl_iostream_uart_config_t *config)
{
  Ecode_t ecode;
  unsigned int allocated_channel;

  // Configure iostream struct and context
  memset(context, 0, sizeof(*context));
  context->rx_dma.cfg = config->rx_dma_cfg;
  context->tx_dma.cfg = config->tx_dma_cfg;
  context->rx_buffer = config->rx_buffer;
  context->rx_buffer_len = config->rx_buffer_length;
  context->rx_read_ptr = context->rx_buffer;
  context->lf_to_crlf = config->lf_to_crlf;
  context->sw_flow_control = config->sw_flow_control;
  context->ctrl_char_scan_ptr = context->rx_read_ptr;
  context->xon = true;
  context->rx_empty = true;
  context->uart_periph = config->uart_periph;
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  context->enable_high_frequency = config->enable_high_frequency;
#endif
  uart->stream.context = context;
  uart->stream.write = uart_write;
  uart->stream.write_async = uart_write_async;
  uart->stream.read = uart_read;
  uart->set_auto_cr_lf = set_auto_cr_lf;
  uart->get_auto_cr_lf = get_auto_cr_lf;
  uart->deinit = uart_deinit;

  // Init the LDMA
  ecode = DMADRV_Init();
  if (ecode != ECODE_OK && ecode != ECODE_EMDRV_DMADRV_ALREADY_INITIALIZED) {
    return SL_STATUS_INITIALIZATION;
  }
  // Allocate the Rx LDMA channel
  ecode = DMADRV_AllocateChannel(&allocated_channel, NULL);
  if (ecode != ECODE_OK) {
    return SL_STATUS_INITIALIZATION;
  }
  context->rx_dma.channel = (uint8_t)allocated_channel;
  // Allocate the Tx LDMA channel
  ecode = DMADRV_AllocateChannel(&allocated_channel, NULL);
  if (ecode != ECODE_OK) {
    DMADRV_FreeChannel(context->rx_dma.channel);
    return SL_STATUS_INITIALIZATION;
  }
  context->tx_dma.channel = (uint8_t)allocated_channel;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  uart->set_read_block = set_read_block;
  uart->get_read_block = get_read_block;
  context->block = true;

  osMutexAttr_t m_attr;
  m_attr.name = "Read Lock";
  m_attr.attr_bits = 0u;
  m_attr.cb_mem = context->read_lock_cb;
  m_attr.cb_size = osMutexCbSize;
  context->read_lock = osMutexNew(&m_attr);
  EFM_ASSERT(context->read_lock != NULL);

  m_attr.name = "Write Lock";
  m_attr.attr_bits = 0u;
  m_attr.cb_mem = context->write_lock_cb;
  context->write_lock = osMutexNew(&m_attr);
  EFM_ASSERT(context->write_lock != NULL);

  osEventFlagsAttr_t f_attr;
  f_attr.name = "RX Data Available Flag";
  f_attr.attr_bits = 0u;
  f_attr.cb_mem = context->rx_data_flag_cb;
  f_attr.cb_size = osEventFlagsCbSize;
  context->rx_data_flag = osEventFlagsNew(&f_attr);
  EFM_ASSERT(context->rx_data_flag != NULL);

#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  #if !defined(SL_CATALOG_KERNEL_PRESENT)
  uart->sleep_on_isr_exit = sleep_on_isr_exit;
  context->sleep = SL_POWER_MANAGER_IGNORE;
  #endif // SL_CATALOG_KERNEL_PRESENT

  uart->set_rx_energy_mode_restriction = set_rx_energy_mode_restriction;
  uart->get_rx_energy_mode_restriction = get_rx_energy_mode_restriction;
  context->em_req_added = false;
  context->tx_idle = true;
  set_rx_energy_mode_restriction(context, config->rx_when_sleeping);
  NVIC_ClearPendingIRQ(config->uart_periph->tx_irq_number);
  NVIC_EnableIRQ(config->uart_periph->tx_irq_number);
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

  // Pause the DMA before starting it to protect against it getting filled up before calling set_new_data_detect.
  ecode = DMADRV_PauseTransfer(context->rx_dma.channel);
  if (ecode != ECODE_OK) {
    return SL_STATUS_INITIALIZATION;
  }

  // Start the (L)DMA to handle RXDATAV
  ecode = DMADRV_PeripheralMemory(context->rx_dma.channel,
                                  context->rx_dma.cfg.xfer_cfg.IOSTREAM_LDMA_TFER_CFG_REQ_SEL,
                                  context->rx_buffer,
                                  context->rx_dma.cfg.src,
                                  true,
                                  context->rx_buffer_len,
                                  dmadrvDataSize1,
                                  __uart_async_rx_dma_callback,
                                  context);
  if (ecode != ECODE_OK) {
    return SL_STATUS_INITIALIZATION;
  }

  // Disable DoneIEN (safe to do here, UART periph not yet started)
  LDMA_PERIPH->CH_CLR[context->rx_dma.channel].CTRL = LDMA_CH_CTRL_DONEIEN;

  NVIC_ClearPendingIRQ(config->uart_periph->rx_irq_number);
  NVIC_EnableIRQ(config->uart_periph->rx_irq_number);

  sl_slist_init(&context->pending_write_ops);

  sl_iostream_set_system_default(&uart->stream);

  // Detect when new data arrives from the bus
  set_new_data_detect(context);

  ecode = DMADRV_ResumeTransfer(context->rx_dma.channel);
  if (ecode != ECODE_OK) {
    return SL_STATUS_INITIALIZATION;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Subscribe to receive a callback when new data is received.
 ******************************************************************************/
sl_status_t sli_iostream_uart_subscribe_to_new_data(sl_iostream_uart_t *iostream_uart,
                                                    sl_iostream_uart_new_data_callback_t callback,
                                                    void *callback_data)
{
  CORE_DECLARE_IRQ_STATE;

  if (iostream_uart == NULL || callback == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)iostream_uart->stream.context;

  if (uart_context->rx_subscriber.callback) {
    return SL_STATUS_BUSY;
  }

  CORE_ENTER_ATOMIC();
  uart_context->rx_subscriber.callback_data = callback_data;
  uart_context->rx_subscriber.callback = callback;
  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Unsubscribe to the new data callback.
 ******************************************************************************/
sl_status_t sli_iostream_uart_unsubscribe_to_new_data(sl_iostream_uart_t *iostream_uart)
{
  CORE_DECLARE_IRQ_STATE;

  if (iostream_uart == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)iostream_uart->stream.context;

  CORE_ENTER_ATOMIC();
  uart_context->rx_subscriber.callback = NULL;
  uart_context->rx_subscriber.callback_data = NULL;
  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set new data detect to wake from sleep.
 ******************************************************************************/
void sl_iostream_uart_prepare_for_sleep(sl_iostream_uart_t *iostream_uart)
{
  set_new_data_detect(iostream_uart->stream.context);
}

/***************************************************************************//**
 * Ensure new data detect is cleared when waking from sleep and data is available.
 ******************************************************************************/
void sl_iostream_uart_wakeup(sl_iostream_uart_t *iostream_uart)
{
  Ecode_t ecode;
  const uint8_t *write_ptr;
  const sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)iostream_uart->stream.context;
  const uint8_t channel = uart_context->rx_dma.channel;

  ecode = DMADRV_PauseTransfer(channel);
  EFM_ASSERT(ecode == ECODE_OK);

  write_ptr = (uint8_t*)LDMA_PERIPH->CH[channel].DST;

  // Handle spurious reset, i.e. not from a byte coming from UART
  if (write_ptr == &null_byte) {
    if (__rx_buffer_full(uart_context)) {
      // Prior to going to sleep, the RX buffer was completely filled. Disable the
      // data-detection and set the LDMA to the done state. This way, the rest of
      // the core will identify the RX buffer as being full.
      CORE_ATOMIC_SECTION(
        LDMA_PERIPH->IEN_CLR = 1 << channel;
        LDMA_PERIPH->CHDIS_SET = 1 << channel;
        LDMA_PERIPH->CHDONE_SET = 1 << channel;
        )
    } else if (!__rx_buffer_empty(uart_context)) {
      // Prior to going to sleep, the RX buffer had some data pending to be read.
      // Disable the data-detection and set the LDMA to its previous state by
      // linking to the resume descriptor. In other words, just resume the
      // reception as per before sleep.
      // Note: in the RX buffer full scenario, linking would have put the LDMA in
      // error since the LDMA doesn't link anywhere, hence the special case.
      LDMA_PERIPH->LINKLOAD = 1 << channel;
    }
  }

  ecode = DMADRV_ResumeTransfer(channel);
  EFM_ASSERT(ecode == ECODE_OK);
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_CATALOG_KERNEL_PRESENT)
/**************************************************************************//**
 * Check if MCU was woken up by new data on UART.
 *****************************************************************************/
static bool wakeup_from_rx(const sl_iostream_uart_context_t *uart_context)
{
  Ecode_t ecode;
  const uint8_t *write_ptr;
  const uint8_t channel = uart_context->rx_dma.channel;
  bool rx_wakeup;

  ecode = DMADRV_PauseTransfer(channel);
  EFM_ASSERT(ecode == ECODE_OK);

  write_ptr = (uint8_t*)LDMA_PERIPH->CH[channel].DST;

  // If the bus was the source of the IRQ that just woke the core up, the DMA should
  // no longer be waiting on the null_byte, since the DMA will have received at
  // least one byte.
  rx_wakeup = !(write_ptr == &null_byte);

  ecode = DMADRV_ResumeTransfer(channel);
  EFM_ASSERT(ecode == ECODE_OK);

  return rx_wakeup;
}

/**************************************************************************//**
 * @brief On ISR exit
 *****************************************************************************/
static sl_power_manager_on_isr_exit_t sleep_on_isr_exit(void *context)
{
  if (context == NULL) {
    return SL_POWER_MANAGER_IGNORE;
  }

  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  if (wakeup_from_rx(context)) {
    // MCU was woken-up from RX byte, wakeup from sleep.
    uart_context->sleep = SL_POWER_MANAGER_WAKEUP;
  }

  if ((uart_context->tx_idle) && (uart_context->sleep == SL_POWER_MANAGER_SLEEP)) {
    sl_power_manager_on_isr_exit_t sleep = uart_context->sleep;
    uart_context->sleep = SL_POWER_MANAGER_IGNORE;
    return sleep;
  } else if (uart_context->sleep == SL_POWER_MANAGER_WAKEUP) {
    sl_power_manager_on_isr_exit_t sleep = uart_context->sleep;
    uart_context->sleep = SL_POWER_MANAGER_IGNORE;
    return sleep;
  } else {
    return SL_POWER_MANAGER_IGNORE;
  }
}
#endif

/**************************************************************************//**
 * Set LF to CRLF conversion
 *****************************************************************************/
static void set_auto_cr_lf(void *context,
                           bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  sl_atomic_store(uart_context->lf_to_crlf, on);
}

/**************************************************************************//**
 * Get LF to CRLF conversion
 *****************************************************************************/
static bool get_auto_cr_lf(void *context)
{
  const sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool conversion;

  sl_atomic_load(conversion, uart_context->lf_to_crlf);

  return conversion;
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**************************************************************************//**
 * Set Rx when sleeping
 *****************************************************************************/
static void set_rx_energy_mode_restriction(void *context,
                                           bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  if (on
      && !uart_context->em_req_added
      && uart_context->enable_high_frequency) {
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
    uart_context->em_req_added = true;
  } else if (!on
             && uart_context->em_req_added) {
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
    uart_context->em_req_added = false;
  }
  CORE_EXIT_ATOMIC();
}
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**************************************************************************//**
 * Get Rx when sleeping
 *****************************************************************************/
static bool get_rx_energy_mode_restriction(void *context)
{
  const sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool em_req_added;

  sl_atomic_load(em_req_added, uart_context->em_req_added);
  return em_req_added;
}
#endif

#if (defined(SL_CATALOG_KERNEL_PRESENT))
/**************************************************************************//**
 * Set read blocking mode
 *****************************************************************************/
static void set_read_block(void *context,
                           bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool block;
  osKernelState_t state;
  osStatus_t status;
  uint32_t set_flags;

  sl_atomic_load(block, uart_context->block);
  state = osKernelGetState();
  if (state == osKernelRunning) {
    // When re-enabling block mode, re-initialize the event flag to reflect the current state
    if (on == true && block == false) {
      // Read-signal should have been deinit when non-blocking
      EFM_ASSERT(uart_context->rx_data_flag == NULL);

      // Init the rx_data_flag boolean event flag
      osEventFlagsAttr_t f_attr;
      f_attr.name = "RX Data Available Flag";
      f_attr.attr_bits = 0u;
      f_attr.cb_mem = uart_context->rx_data_flag_cb;
      f_attr.cb_size = osEventFlagsCbSize;
      uart_context->rx_data_flag = osEventFlagsNew(&f_attr);
      EFM_ASSERT(uart_context->rx_data_flag != NULL);

      // Set the event flag to reflect the current state
      if (!rx_buffer_empty(uart_context)) {
        set_flags = osEventFlagsSet(uart_context->rx_data_flag, RX_DATA_AVAILABLE_FLAG);
        EFM_ASSERT(set_flags == RX_DATA_AVAILABLE_FLAG);
      }
    }
    // When disabling block mode, deinit the rx_data_flag event flag
    else if (on == false && block == true) {
      status = osEventFlagsDelete(uart_context->rx_data_flag);
      EFM_ASSERT(status == osOK);
      uart_context->rx_data_flag = NULL;
    }
  }
  // Set the block context variable
  sl_atomic_store(uart_context->block, on);
}
#endif

#if (defined(SL_CATALOG_KERNEL_PRESENT))
/**************************************************************************//**
 * Get read blocking mode
 *****************************************************************************/
static bool get_read_block(void *context)
{
  const sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool block;

  sl_atomic_load(block, uart_context->block);
  return block;
}
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
/**************************************************************************//**
 * Signal transmit complete
 *****************************************************************************/
void sli_uart_txc(void *context)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  if (uart_context->tx_idle == false) {
    EFM_ASSERT(uart_context->uart_periph->tx_completed != NULL);
    uart_context->uart_periph->tx_completed(context, false);
    uart_context->tx_idle = true;
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#if !defined(SL_CATALOG_KERNEL_PRESENT)
    uart_context->sleep = SL_POWER_MANAGER_SLEEP;
#endif
  }
  CORE_EXIT_ATOMIC();
}
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * UART Stream De-init
 ******************************************************************************/
static sl_status_t uart_deinit(void *stream)
{
  sl_iostream_uart_t *uart = (sl_iostream_uart_t *)stream;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)uart->stream.context;
  sl_iostream_t *default_stream;
  sl_status_t status = SL_STATUS_OK;
  Ecode_t ecode = ECODE_OK;

  if (uart_context->async_tx_mode) {
    EFM_ASSERT(false);
    return SL_STATUS_NOT_SUPPORTED;
  }

  sli_iostream_uart_unsubscribe_to_new_data(uart);

#if (defined(SL_CATALOG_KERNEL_PRESENT))
  if (osKernelGetState() == osKernelRunning) {
    // Acquire locks to ensure no others task try to perform operation on the stream at sametime
    EFM_ASSERT(osMutexAcquire(uart_context->write_lock, osWaitForever) == osOK);  // If deinit is called twice in a
                                                                                  // row, the assert will trigger
    // Bypass lock if we print before the kernel is running
    EFM_ASSERT(osMutexAcquire(uart_context->read_lock, osWaitForever) == osOK);   // If deinit is called twice in a
                                                                                  //row, the assert will trigger
  }
#endif

  default_stream = sl_iostream_get_default();

  // Check if uart stream is the default and if it's the case,
  // remove it's reference as the default
  if ((sl_iostream_uart_t*)default_stream == uart) {
    sl_iostream_set_system_default(NULL);
  }

  NVIC_ClearPendingIRQ(uart_context->uart_periph->rx_irq_number);
  NVIC_DisableIRQ(uart_context->uart_periph->rx_irq_number);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  NVIC_ClearPendingIRQ(uart_context->uart_periph->tx_irq_number);
  NVIC_DisableIRQ(uart_context->uart_periph->tx_irq_number);
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Delete Kernel synchronization objects.
  if (uart_context->block) {
    status = osEventFlagsDelete(uart_context->rx_data_flag);
    EFM_ASSERT(status == osOK);
  }

  status = osMutexDelete(uart_context->read_lock);
  EFM_ASSERT(status == osOK);

  status = osMutexDelete(uart_context->write_lock);
  EFM_ASSERT(status == osOK);
#endif

  // Stop the DMA
  ecode = DMADRV_StopTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  // Free the DMA channel
  ecode = DMADRV_FreeChannel(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  // Try to deinit the DMADRV
  ecode = DMADRV_DeInit();
  EFM_ASSERT(ecode == ECODE_OK || ecode == ECODE_EMDRV_DMADRV_IN_USE);

  // Clear iostream struct and context
  uart->stream.context = NULL;
  uart->stream.write = NULL;
  uart->stream.read = NULL;
  uart->set_auto_cr_lf = NULL;
  uart->get_auto_cr_lf = NULL;

  status = uart_context->uart_periph->deinit(uart_context);

  return status;
}

/***************************************************************************//**
 * Scan the RX buffer in reverse to find a new control character.
 ******************************************************************************/
static void scan_for_ctrl_char(sl_iostream_uart_context_t * uart_context)
{
  // Since RX is buffered, everytime a TX is attempted, a check must be made to validate if a control
  // character was received since the last TX call. Check between the last scanned data, and the
  // newest data received from the bus.
  //
  // [scanned data| unscanned data | reception room ]
  //      scan ptr^     newest byte^
  //               <=====Scan======
  //
  // Scan in reverse order, as only the latest control character is of interest. For example, if
  // [XOFF,XOFF,XON] is received, only the last XON byte is to be considered. Once scan is complete,
  // move the scan pointer to the end of the newest data section so that the next scan will only check
  // unscanned data, making the check faster.
  //
  // [     scanned unread data     | reception room ]
  //                       scan ptr^
  //                    newest byte^
  //
  // Note: The scan pointer will also be updated when RX buffer is read by user to accelerate the scan.

  const uint8_t *current_byte;
  uint8_t *newest_byte;
  bool rx_empty;
  Ecode_t ecode;
  CORE_DECLARE_IRQ_STATE;

  // No data to be scanned
  if (rx_buffer_empty(uart_context)) {
    EFM_ASSERT(uart_context->ctrl_char_scan_ptr == uart_context->rx_read_ptr);
    return;
  }

  CORE_ENTER_ATOMIC();
  ecode = DMADRV_PauseTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  // Get the newest byte received
  newest_byte = __get_write_ptr(uart_context) - 1;
  if (newest_byte < uart_context->rx_buffer) {
    // Wrap newest byte ptr around the ring buffer
    newest_byte = uart_context->rx_buffer + (uart_context->rx_buffer_len - 1);
  }

  rx_empty = uart_context->rx_empty;

  ecode = DMADRV_ResumeTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  // No data to be scanned
  if (rx_empty) {
    // Sanity check
    EFM_ASSERT(uart_context->ctrl_char_scan_ptr == uart_context->rx_read_ptr);

    goto exit;
  }

  // Scan the entire buffer until we reach the position of the last scan,
  // or until we find the newest control character
  current_byte = newest_byte;
  while (current_byte != uart_context->ctrl_char_scan_ptr) {
    if (*current_byte == UARTXON || *current_byte == UARTXOFF) {
      // Found latest control character, apply it and break early.
      sl_atomic_store(uart_context->xon, (*current_byte == UARTXON));
      break;
    }

    // Decrement and wrap current byte ptr around the ring buffer
    current_byte--;
    if (current_byte < uart_context->rx_buffer) {
      current_byte = uart_context->rx_buffer + (uart_context->rx_buffer_len - 1);
    }
  }

  // Update scan pointer
  uart_context->ctrl_char_scan_ptr = newest_byte;

  exit:
  CORE_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t nolock_uart_write(void *context,
                                     const void *buffer,
                                     size_t buffer_length)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  char *c = (char *)buffer;
  bool lf_to_crlf = false;
  sl_status_t status = SL_STATUS_FAIL;

  sl_atomic_load(lf_to_crlf, uart_context->lf_to_crlf);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (uart_context->tx_idle == true && uart_context->enable_high_frequency) {
    uart_context->tx_idle = false;
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
  }
  CORE_EXIT_ATOMIC();
#endif

  uint32_t i = 0;
  while (i < buffer_length) {
    bool xon = false;
    if (uart_context->sw_flow_control == true) {
      scan_for_ctrl_char(uart_context);
    }
    sl_atomic_load(xon, uart_context->xon);
    if (xon) {
      if ((lf_to_crlf == true) && (*c == '\n')) {
        status = uart_context->uart_periph->tx(uart_context, '\r');
        if (status != SL_STATUS_OK) {
          return status;
        }
      }
      status = uart_context->uart_periph->tx(uart_context, *c);
      if (status != SL_STATUS_OK) {
        return status;
      }
      c++;
      i++;
    }         // Active wait if xon is false
  }

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
  uart_context->uart_periph->tx_completed(context, true);
#endif

  return status;
}

/***************************************************************************//**
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t uart_write(void *context,
                              const void *buffer,
                              size_t buffer_length)
{
  const sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  if (uart_context->async_tx_mode) {
    return SL_STATUS_NOT_AVAILABLE;         // Can't do synchronous calls in async TX mode
  }

#if (defined(SL_CATALOG_KERNEL_PRESENT))
  osStatus_t status;
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    status = osMutexAcquire(uart_context->write_lock, osWaitForever);

    if (status != osOK) {
      return SL_STATUS_INVALID_STATE;       // Can happen if a task deinit and another try to write at sametime
    }
  }
#endif

  nolock_uart_write(context, buffer, buffer_length);

#if (defined(SL_CATALOG_KERNEL_PRESENT))
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    status = osMutexRelease(uart_context->write_lock);
    EFM_ASSERT(status == osOK);
  }
#endif
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Callback function for UART asynchronous TX DMA completion.
 ******************************************************************************/
static bool __uart_async_tx_dma_callback(unsigned int channel, unsigned int sequenceNo, void *arg)
{
  (void)channel;
  (void)sequenceNo;
  sl_slist_node_t *node;
  sli_iostream_write_async_op_t *async_op;
  sli_iostream_write_async_op_t *completed_async_op = (sli_iostream_write_async_op_t *)arg;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)completed_async_op->context;

  EFM_ASSERT(uart_context->async_transfer_in_progress);

  uart_context->async_transfer_in_progress = false;
  node = sl_slist_pop(&uart_context->pending_write_ops);
  if (node != NULL) {
    async_op = SL_SLIST_ENTRY(node, sli_iostream_write_async_op_t, node);
    __uart_async_start_write(async_op);
  }

  if (completed_async_op->on_write_completed != NULL) {
    completed_async_op->on_write_completed(completed_async_op, SL_STATUS_OK, completed_async_op->on_write_completed_arg);
  }

  return false;
}

/***************************************************************************//**
 * Start UART asynchronous write operation (must be called in atomic section).
 *****************************************************************************/
static void __uart_async_start_write(sli_iostream_write_async_op_t *async_op)
{
  Ecode_t ecode;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)async_op->context;

  uart_context->tx_dma.desc = (LDMA_Descriptor_t)
                              IOSTREAM_LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(async_op->buffer,
                                                                       uart_context->tx_dma.cfg.dst,
                                                                       async_op->buffer_length);

  uart_context->async_transfer_in_progress = true;
  ecode = DMADRV_LdmaStartTransfer(uart_context->tx_dma.channel,
                                   &uart_context->tx_dma.cfg.xfer_cfg,
                                   &uart_context->tx_dma.desc,
                                   __uart_async_tx_dma_callback,
                                   async_op);
  EFM_ASSERT(ecode == ECODE_OK);
}

/***************************************************************************//**
 * Perform UART asynchronous write operation.
 ******************************************************************************/
static sl_status_t uart_write_async(void *context, sli_iostream_write_async_op_t *async_op)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  if (!uart_context->async_tx_mode) {
    return SL_STATUS_NOT_AVAILABLE;
  }

  if (async_op->buffer_length > IOSTREAM_LDMA_MAX_XFER_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  async_op->context = uart_context;

  if (uart_context->async_transfer_in_progress) {
    sl_slist_push_back(&uart_context->pending_write_ops, &async_op->node);
  } else {
    __uart_async_start_write(async_op);
  }

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Internal stream read implementation
 ******************************************************************************/
static sl_status_t uart_read(void *context,
                             void *buffer,
                             size_t buffer_length,
                             size_t *bytes_read)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  size_t rx_len;

  #if (defined(SL_CATALOG_KERNEL_PRESENT))
  osStatus_t status;
  uint32_t set_flags;
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    status = osMutexAcquire(uart_context->read_lock, osWaitForever);

    if (status != osOK) {
      return SL_STATUS_INVALID_STATE;   // Can happen if a task deinit and another try to read at same time
    }

    // Need to check if data is available, as DMA can be started before the kernel was
    // able to be started, meaning the flag will not be set even when data is available.
    if (uart_context->block) {
      // osFlagsNoClear used to unlock directly if data is still available. Flag cleared
      // when no more data is available to be read.
      set_flags = osEventFlagsWait(uart_context->rx_data_flag, RX_DATA_AVAILABLE_FLAG, osFlagsNoClear, osWaitForever);
      EFM_ASSERT(set_flags == RX_DATA_AVAILABLE_FLAG);
    }
  }
  #endif

  *bytes_read = 0;

  while ((rx_len = read_rx_buffer(uart_context,
                                  ((uint8_t *)buffer + *bytes_read),
                                  (buffer_length - *bytes_read)))) {
    // Read the RX buffer until it has been emptied, or the user buffer
    // has been filled.
    *bytes_read += rx_len;

    if (*bytes_read >= buffer_length) {
      // Exit path in case previous call overflowed.
      break;
    }
  }

  #if (defined(SL_CATALOG_KERNEL_PRESENT))
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    EFM_ASSERT(osMutexRelease(uart_context->read_lock) == osOK);
  }
  #endif

  if (*bytes_read == 0) {
    return SL_STATUS_EMPTY;
  } else {
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Returns whether the RX buffer is full.
 *
 * @note Make sure the DMA is paused before calling this API.
 ******************************************************************************/
static inline bool __rx_buffer_full(const sl_iostream_uart_context_t *uart_context)
{
  const uint8_t *write_ptr = (uint8_t *)LDMA_PERIPH->CH[uart_context->rx_dma.channel].DST;
  const bool dma_linked = LDMA_PERIPH->CH[uart_context->rx_dma.channel].LINK & LDMA_CH_LINK_LINK;
  Ecode_t ecode;
  bool dma_done;

  ecode = DMADRV_TransferDone(uart_context->rx_dma.channel, &dma_done);
  EFM_ASSERT(ecode == ECODE_OK);

  // When the DMA is done, it means that the RX buffer was filled up. However, if
  // we went to sleep while the RX buffer was full, and the DMA is woken-up by something
  // other than the core, the DMA will not be done. Instead, it will be pointing to the
  // null_byte, but it wont be pointing to another descriptor, as the RX buffer has
  // no more room.
  return (dma_done || ((write_ptr == &null_byte) && !dma_linked));
}

/***************************************************************************//**
 * Returns whether the DMA is in data detect mode.
 *
 * @note Caller must pause the DMA prior this calling this.
 ******************************************************************************/
static inline bool __new_data_detect_armed(const sl_iostream_uart_context_t *uart_context)
{
  const uint8_t *write_ptr = (uint8_t *)LDMA_PERIPH->CH[uart_context->rx_dma.channel].DST;

  return write_ptr == &null_byte;
}

/***************************************************************************//**
 * Returns whether the RX buffer is empty.
 *
 * @note Caller must pause the DMA prior this calling this.
 ******************************************************************************/
static inline bool __rx_buffer_empty(const sl_iostream_uart_context_t *uart_context)
{
  // No data to be read if DMA is writing to null_byte, as it is waiting on new
  // data from the bus.
  const uint8_t *write_ptr = (uint8_t *)LDMA_PERIPH->CH[uart_context->rx_dma.channel].DST;
  const bool dma_linked = LDMA_PERIPH->CH[uart_context->rx_dma.channel].LINK & LDMA_CH_LINK_LINK;

  if ((write_ptr == &null_byte) && dma_linked) {
    // When the RX buffer becomes empty, the DMA is in data detect mode, and it should point to
    // rx_read_ptr.
    write_ptr = (uint8_t *)uart_context->rx_dma.rx_resume_desc.xfer.IOSTREAM_LDMA_DESCRIPTOR_DST_ADDR;

    return (write_ptr == uart_context->rx_read_ptr);
  }

  return false;
}

/***************************************************************************//**
 * Checks whether the RX buffer is empty with the LDMA paused.
 ******************************************************************************/
static inline bool rx_buffer_empty(const sl_iostream_uart_context_t *uart_context)
{
  bool empty;
  Ecode_t ecode = DMADRV_PauseTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  empty = __rx_buffer_empty(uart_context);

  ecode = DMADRV_ResumeTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  return empty;
}

/***************************************************************************//**
 * Get the next byte to be written to by the (L)DMA.
 ******************************************************************************/
static inline uint8_t* __get_write_ptr(const sl_iostream_uart_context_t *uart_context)
{
  bool dma_full = __rx_buffer_full(uart_context);
  uint8_t* write_ptr = NULL;

  if (dma_full) {
    // When the DMA is completely done, it has wrapped over the circular buffer
    // and filled it up completely.
    write_ptr = uart_context->rx_read_ptr;
  } else {
    write_ptr = (uint8_t *)LDMA_PERIPH->CH[uart_context->rx_dma.channel].DST;
  }

  // Sanity check for buffer over/underflow
  EFM_ASSERT(write_ptr <= (uart_context->rx_buffer + uart_context->rx_buffer_len)
             && write_ptr >= uart_context->rx_buffer);

  // Wrap dst around
  if (write_ptr == (uart_context->rx_buffer + uart_context->rx_buffer_len)) {
    write_ptr = uart_context->rx_buffer;
  }

  return write_ptr;
}

/***************************************************************************//**
 * Compute how many bytes available to read in UART ring buffer.
 *
 * @note Caller must ensure that rx buffer was not empty prior to calling, or will
 * hit an assert.
 ******************************************************************************/
static inline size_t __get_bytes_available(const sl_iostream_uart_context_t *uart_context)
{
  const uint8_t *write_ptr = __get_write_ptr(uart_context);

  if (write_ptr > uart_context->rx_read_ptr) {
    // Read data between read_ptr and write_ptr
    return write_ptr - uart_context->rx_read_ptr;
  } else {
    // write_ptr wrapped around. Read data from read_ptr to end of buffer.
    // Sanity check that the read pointer didn't overflow.
    EFM_ASSERT(uart_context->rx_read_ptr < (uart_context->rx_buffer + uart_context->rx_buffer_len));

    return (uart_context->rx_buffer + uart_context->rx_buffer_len) - uart_context->rx_read_ptr;
  }
}

/***************************************************************************//**
 * Update ring buffer pointers and DMA descriptor.
 *
 * This function blocks reception on the DMA. It is crucial it completes as soon
 * as possible, otherwise data will be dropped when no flow control is available.
 ******************************************************************************/
static void update_ring_buffer(sl_iostream_uart_context_t * uart_context, size_t read_size)
{
  Ecode_t ecode;
  bool dma_full;
  size_t xfer_cnt;
  uint8_t *write_ptr;
  #if defined(SL_CATALOG_KERNEL_PRESENT)
  uint32_t set_flags;
  #endif

  // Pause the DMA to update its registers
  ecode = DMADRV_PauseTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  dma_full = __rx_buffer_full(uart_context);

  // Get next byte to be written by DMA
  write_ptr = __get_write_ptr(uart_context);

  if (write_ptr > uart_context->rx_read_ptr) {
    // The DMA should never be full here, or else the write_ptr would have
    // wrapped behind the buffer.
    EFM_ASSERT(!dma_full);

    // LDMA is writing ahead of read pointer. Make sure LDMA is wrapped to re-use
    // space made available from current read.
    if (uart_context->rx_read_ptr == uart_context->rx_buffer) {
      // We have just consumed the first byte of the ring buffer, meaning there is now
      // room at the start of the RX buffer for reception.
      // The DMA is still running ahead of the of the read pointer. Populate the wrap
      // descriptor so that the DMA can automatically use the new space once it has
      // reached the end of the buffer:
      // +-----------wrap_desc-----------+
      // v                               |
      // [available_data | reception_room]
      // ^    ^          ^
      // |read|          write_ptr
      // ^
      // read_ptr

      //
      // Once the DMA is resumed, it will keep receiving in the reception room, and
      // wrap around once it has reached the end.
      uart_context->rx_dma.wrap_desc = (LDMA_Descriptor_t) IOSTREAM_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(uart_context->rx_dma.cfg.src,
                                                                                                    uart_context->rx_buffer,
                                                                                                    read_size);
      uart_context->rx_dma.wrap_desc.xfer.IOSTREAM_LDMA_DESCRIPTOR_DONE_IFS = false; // Don't generate an IRQ when DMA get filled-up
      LDMA_PERIPH->CH[uart_context->rx_dma.channel].LINK = (((uintptr_t)&uart_context->rx_dma.wrap_desc) & _LDMA_CH_LINK_LINKADDR_MASK)   // Link to wrap desc
                                                           | _LDMA_CH_LINK_LINK_MASK // Enable link
                                                           | _LDMA_CH_LINK_LINKMODE_ABSOLUTE; // Link absolute
    } else {
      // The wrap descriptor has already been configured, as this is not the first byte
      // of the RX buffer. Simply increase the size of the wrap descriptor with the
      // room we just made available.
      uart_context->rx_dma.wrap_desc.xfer.IOSTREAM_LDMA_DESCRIPTOR_XFER_CNT += read_size;
    }
  } else {
    // The DMA has already wrapped around the buffer using the wrap_desc. Update the
    // DMA registers so the reception room can be increased by the number of bytes
    // we have just read.
    //
    // [available data | reception_room | available_data]
    //                 ^                ^    ^
    //                 write_ptr        |read|
    //                                  ^
    //                                  read_ptr

    if (!dma_full) {
      xfer_cnt = (LDMA_PERIPH->CH[uart_context->rx_dma.channel].CTRL & _LDMA_CH_CTRL_XFERCNT_MASK)
                 >> _LDMA_CH_CTRL_XFERCNT_SHIFT;
      xfer_cnt += read_size;
      // Set xfer_cnt
      LDMA_PERIPH->CH[uart_context->rx_dma.channel].CTRL = (LDMA_PERIPH->CH[uart_context->rx_dma.channel].CTRL & ~_LDMA_CH_CTRL_XFERCNT_MASK)
                                                           | (xfer_cnt << _LDMA_CH_CTRL_XFERCNT_SHIFT);
    } else {
      // LDMA completed before we could update it. Start it again over the space just made available.
      ecode = DMADRV_PeripheralMemory(uart_context->rx_dma.channel,
                                      uart_context->rx_dma.cfg.xfer_cfg.IOSTREAM_LDMA_TFER_CFG_REQ_SEL,
                                      write_ptr,
                                      uart_context->rx_dma.cfg.src,
                                      true,
                                      read_size,
                                      dmadrvDataSize1,
                                      __uart_async_rx_dma_callback,
                                      uart_context);
      EFM_ASSERT(ecode == ECODE_OK);
      // Disable DoneIEN (safe to do here, DMA is paused)
      LDMA_PERIPH->CH[uart_context->rx_dma.channel].CTRL &= ~LDMA_CH_CTRL_DONEIEN;
    }
  }

  // Move the read_ptr to the end of what we have just read.
  // [available_data | reception_room]
  // ^    ^          ^
  // |read|          write_ptr
  //      ^
  //      read_ptr

  uart_context->rx_read_ptr += read_size;
  if (uart_context->rx_read_ptr == (uart_context->rx_buffer + uart_context->rx_buffer_len)) {
    // Wrap the RX buffer around
    uart_context->rx_read_ptr = uart_context->rx_buffer;
  }

  // Sanity check rx_ptr didn't overflow
  EFM_ASSERT(uart_context->rx_read_ptr < (uart_context->rx_buffer + uart_context->rx_buffer_len));

  if (uart_context->rx_read_ptr == write_ptr) {
    // No more data available to be read. Arm the new data detection descriptor.
    // [reception_room]
    //  ^
    //  write_ptr
    //  ^
    //  read_ptr

    #if defined(SL_CATALOG_KERNEL_PRESENT)
    if (uart_context->block) {
      // Unset event flag to wait for new data detection
      set_flags = osEventFlagsClear(uart_context->rx_data_flag, RX_DATA_AVAILABLE_FLAG);
      EFM_ASSERT(set_flags == RX_DATA_AVAILABLE_FLAG);
    }
    #endif

    sl_atomic_store(uart_context->rx_empty, true);

    // All data consumed by user
    set_new_data_detect(uart_context);
  }

  // Resume DMA after update
  ecode = DMADRV_ResumeTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);
}

/***************************************************************************//**
 * Tries to read the requested amount of data.
 * Returns the number of bytes read.
 ******************************************************************************/
static size_t read_rx_buffer(sl_iostream_uart_context_t * uart_context,
                             uint8_t * buffer,
                             size_t buffer_len)
{
  bool update_xon = false;
  bool send_xon = false;
  size_t read_size = 0;
  size_t ret_val = 0;
  CORE_DECLARE_IRQ_STATE;

  if (buffer_len == 0 || buffer == NULL) {
    return 0;
  }

  if (rx_buffer_empty(uart_context)) {
    // RX buffer is empty
    return 0;
  }

  // Compute number of bytes available
  read_size = __get_bytes_available(uart_context);

  // Limit read size to buffer size
  read_size = read_size > buffer_len ? buffer_len : read_size;
  EFM_ASSERT(read_size > 0);

  // Number of bytes written to user buffer can be different if control character are present
  ret_val = read_size;

  // Copy data to ouput buffer
  {
    // Handle control character and copy data to the user buffer
    if (uart_context->sw_flow_control) {
      CORE_ENTER_ATOMIC();

      const uint8_t *curr_char = uart_context->rx_read_ptr;

      // When the RX buffer was filled up, sent a XOFF. Just made some room in the buffer, signal
      // the remote it can resume TX.
      send_xon = __rx_buffer_full(uart_context);

      for (size_t bytes_read = 0; bytes_read < read_size; bytes_read++, curr_char++) {
        if (curr_char == uart_context->ctrl_char_scan_ptr) {
          // Caught up to most recent scanned byte, increment the scan ptr
          uart_context->ctrl_char_scan_ptr++;
          update_xon = true;
        }

        if (*curr_char != (uint8_t)UARTXON && *curr_char != (uint8_t)UARTXOFF) {
          // No control character, read data and increment user buffer
          *buffer++ = *curr_char;
          continue;
        }

        // Received a control character, don't copy it to the user buffer
        ret_val--;

        if (update_xon) {
          // Current byte is the most recent scanned byte. Apply control character
          sl_atomic_store(uart_context->xon, (*curr_char == (uint8_t)UARTXON));
        }
      }

      // Wrap ctrl_char_scan_ptr around the rx_buffer
      if (uart_context->ctrl_char_scan_ptr == (uart_context->rx_buffer + uart_context->rx_buffer_len)) {
        uart_context->ctrl_char_scan_ptr = uart_context->rx_buffer;
      }

      EFM_ASSERT(uart_context->ctrl_char_scan_ptr >= uart_context->rx_buffer
                 && uart_context->ctrl_char_scan_ptr < (uart_context->rx_buffer + uart_context->rx_buffer_len));

      CORE_EXIT_ATOMIC();
    }
    // Copy the data to the output buffer
    else {
      memcpy(buffer, uart_context->rx_read_ptr, read_size);
    }
  }

  // Update the ring buffer after read
  update_ring_buffer(uart_context, read_size);

  if (uart_context->sw_flow_control && send_xon) {
    // Just made some room in RX buffer, signal remote it can resume TX.
    uart_context->uart_periph->tx(uart_context, UARTXON);
  }

  return ret_val;
}

/***************************************************************************//**
 * RX DMA chanel interrupt handler.
 *
 * The following scenarios can cause this handler to fire:
 *  - The data detection mechanism has been triggered.
 *  - The RX buffer has been completely filled up.
 ******************************************************************************/
static bool __uart_async_rx_dma_callback(unsigned int channel, unsigned int sequenceNo,
                                         void *userParam)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *) userParam;
  uint8_t c;
  #if defined(SL_CATALOG_KERNEL_PRESENT)
  uint32_t set_flags;
  osKernelState_t state;
  #endif
  (void) sequenceNo;
  (void) channel;
  (void) uart_context;

  if (uart_context->rx_empty) {
    // Notify registered callback
    if (uart_context->rx_subscriber.callback) {
      uart_context->rx_subscriber.callback(uart_context->rx_subscriber.callback_data);
    }
    uart_context->rx_empty = false;
  }

  if (uart_context->sw_flow_control && __rx_buffer_full(uart_context)) {
    // Remote just filled up RX buffer, signal a stop.
    uart_context->uart_periph->tx(uart_context, UARTXOFF);

    for (uint8_t it = 0; it < MAX_RX_FIFO_DEPTH; it++) {
      if (uart_context->uart_periph->rx(uart_context, (char*) &c) != SL_STATUS_OK) {
        // No more RX data to process.
        break;
      }

      if (c == UARTXON || c == UARTXOFF) {
        // Remote sent control byte when RX buffer is full, handle it.
        sl_atomic_store(uart_context->xon, (c == UARTXON));
      }
    }

    // In SW flow control, need to detect further data
    set_new_data_detect(uart_context);
  }
  uart_context->rx_empty = false;

  // In baremetal, the IRQ handler is used to detect new data and wake the core
  // up from sleep.

  #if defined(SL_CATALOG_KERNEL_PRESENT)
  // In RTOS, the DMA IRQ has the role of posting the rx_data_flag event flag to
  // unlock any pending read task when new data is received.
  if (uart_context->block) {
    // Make sure the kernel is ready before releasing the event flag
    state = osKernelGetState();
    if (state == osKernelRunning || state == osKernelLocked) {
      // When waking up from sleep, kernel can be in Locked state. Make sure
      // to unlock the event flag in order to resume normal operation.
      set_flags = osEventFlagsSet(uart_context->rx_data_flag, RX_DATA_AVAILABLE_FLAG);
      EFM_ASSERT(set_flags == RX_DATA_AVAILABLE_FLAG);
    }
  }
  #endif

  return false;
}

/***************************************************************************//**
 * Detect the arrival of new data coming from the bus.
 ******************************************************************************/
static inline void set_new_data_detect(sl_iostream_uart_context_t *uart_context)
{
  LDMA_Descriptor_t *data_detect_desc = &uart_context->rx_dma.data_detect_desc;
  LDMA_Descriptor_t *resume_desc = &uart_context->rx_dma.rx_resume_desc;
  uint8_t *write_ptr;
  bool dma_done;
  Ecode_t ecode;
  int count;

  *data_detect_desc = (LDMA_Descriptor_t)IOSTREAM_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&null_byte,
                                                                                  &null_byte,
                                                                                  1);

  // The new data detection mechanism works by starting a dummy transfer on a
  // null byte. This transfer is started by the RXDATAV signal coming from the
  // UART peripheral. When new data is received, an LDMA IRQ will fire, waking
  // the core up from sleep and/or releasing the rx_data_flag event flag. Note that
  // no data is transferred from the UART peripheral on this transfer. Its sole use
  // is to generate an IRQ in order to signal the system that new data is available
  // to be read.
  //
  // The DMA will then automatically link to the next position it was receiving
  // from, resuming normal operation. If no more room was available for reception,
  // it will simply stay in the done state. This approach was chosen over using the
  // UART peripheral's IRQ, because as soon as the RXDATAV signal is asserted, the
  // DMA will consume the data byte, making it impossible to identify the source
  // of the IRQ. Also, since we already have a DMA channel, this solution uses
  // no extra hardware. Finally, using a NULL byte was more suitable than using
  // the actual RX buffer as a transfer, as if the RX buffer is full when attempting
  // to detect new data coming from the bus, we would overwrite existing data. This
  // scenario is possible when going to sleep with a full RX buffer.
  //
  // After this function, the DMA chain will look like the following:
  //     [detect_data]
  //                 v
  // [available_data | reception_room]
  // ^               ^
  // read_ptr        write_ptr


  // Pause the DMA to update it
  ecode = DMADRV_PauseTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);

  *data_detect_desc = (LDMA_Descriptor_t)IOSTREAM_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&null_byte,
                                                                                  &null_byte,
                                                                                  1);

  if (__new_data_detect_armed(uart_context)) {
    // DMA is already waiting on new data to arrive, nothing to be done.
    ecode = DMADRV_ResumeTransfer(uart_context->rx_dma.channel);
    EFM_ASSERT(ecode == ECODE_OK);
    return;
  }

  // Check if DMA is running (note: pause != done)
  ecode = DMADRV_TransferDone(uart_context->rx_dma.channel, &dma_done);
  EFM_ASSERT(ecode == ECODE_OK);

  if (!dma_done) {
    // DMA is running. Save its current state in the rx_resume_desc and link the
    // data_detect_desc to it so normal operation can resume once data has been
    // received.
    write_ptr = __get_write_ptr(uart_context);
    ecode = DMADRV_TransferRemainingCount(uart_context->rx_dma.channel, &count);
    EFM_ASSERT(ecode == ECODE_OK);
    // DMA is running, count should be greater than zero.
    EFM_ASSERT(count > 0);

    // Set idle desc to current LDMA state
    *resume_desc = (LDMA_Descriptor_t)IOSTREAM_LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(uart_context->rx_dma.cfg.src,
                                                                               write_ptr,
                                                                               count);
    resume_desc->xfer.link = (LDMA_PERIPH->CH[uart_context->rx_dma.channel].LINK & _LDMA_CH_LINK_LINK_MASK) >> _LDMA_CH_LINK_LINK_SHIFT;
    resume_desc->xfer.IOSTREAM_LDMA_DESCRIPTOR_DONE_IFS = false;
    resume_desc->xfer.IOSTREAM_LDMA_DESCRIPTOR_LINK_MODE = (LDMA_PERIPH->CH[uart_context->rx_dma.channel].LINK & _LDMA_CH_LINK_LINKMODE_MASK)
                                                           >> _LDMA_CH_LINK_LINKMODE_SHIFT;
    resume_desc->xfer.IOSTREAM_LDMA_DESCRIPTOR_LINK_ADDR = (LDMA_PERIPH->CH[uart_context->rx_dma.channel].LINK & _LDMA_CH_LINK_LINKADDR_MASK)
                                                           >> _LDMA_CH_LINK_LINKADDR_SHIFT;

    // Link data_detect_desc to rx_resume_desc to resume normal operation
    data_detect_desc->xfer.link = true;
    data_detect_desc->xfer.IOSTREAM_LDMA_DESCRIPTOR_LINK_MODE = LDMA_CH_LINK_LINKMODE_ABSOLUTE;
    data_detect_desc->xfer.IOSTREAM_LDMA_DESCRIPTOR_LINK_ADDR = IOSTREAM_LDMA_DESCRIPTOR_LINKABS_ADDR_TO_LINKADDR(&uart_context->rx_dma.rx_resume_desc);
  } else {
    // DMA is not running. Make sure the data_detect_desc is not linking anywhere,
    // as once data will be detected, we will have nowhere to store it. This case
    // is possible when waking up from sleep with a full RX buffer.
    // Note: when this completes, the DST pointer will point to null_byte+1, which
    // is not valid. However, the DMA will be in the DONE state, which will indicate
    // that the buffer is full.
    data_detect_desc->xfer.link = false;
  }

  // Start the DMA on the idle descriptor
  ecode = DMADRV_LdmaStartTransfer(uart_context->rx_dma.channel,
                                   &uart_context->rx_dma.cfg.xfer_cfg,
                                   data_detect_desc,
                                   __uart_async_rx_dma_callback,
                                   uart_context);
  EFM_ASSERT(ecode == ECODE_OK);

  // Resume DMA
  ecode = DMADRV_ResumeTransfer(uart_context->rx_dma.channel);
  EFM_ASSERT(ecode == ECODE_OK);
}
