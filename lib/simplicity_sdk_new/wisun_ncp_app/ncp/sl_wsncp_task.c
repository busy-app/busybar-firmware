/***************************************************************************//**
 * @file sl_wsncp_task.c
 * @brief Wi-SUN NCP task
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "sl_assert.h"
#include <cmsis_os2.h>
#include <inttypes.h>
#include "sl_wisun_events.h"
#include "sl_wisun_msg_api.h"
#include "sl_wsncp_interface.h"
#include "sl_wsncp_task.h"
#include "sl_wisun_common.h"
#include "sl_sleeptimer.h"
#include "sl_wisun_trace_api.h"
#include "em_core.h"
#include "sl_wsncp_buffer.h"

// Configuration-aware defines - use .slcp values if available, otherwise use defaults
#ifndef SL_WSNCP_TASK_PRIORITY
#define SL_WSNCP_TASK_PRIORITY osPriorityNormal
#endif

#ifndef SL_WSNCP_TASK_STACK_SIZE
#define SL_WSNCP_TASK_STACK_SIZE       500 // in units of CPU_INT32U
#endif

#ifndef SL_WSNCP_MAX_TIMEOUT_COUNT
#define SL_WSNCP_MAX_TIMEOUT_COUNT     1000
#endif

// Task flag definitions (these are internal and don't need to be configurable)
#define SL_WSNCP_TASK_FLAG_REQ_READY   1
#define SL_WSNCP_TASK_FLAG_IND_READY   2
#define SL_WSNCP_TASK_FLAG_COMM_READY  4
#define SL_WSNCP_TASK_FLAG_IND_DONE    8


static sl_wsncp_buffer_t sl_wsncp_req;
static sl_wsncp_buffer_t sl_wsncp_cnf;
static sl_wsncp_buffer_t sl_wsncp_ind;

osThreadId_t sl_wsncp_task_id;
osEventFlagsId_t sl_wsncp_task_flags;

/// Tick count when TX was initiated
static uint32_t ncp_timestamp_tx;

/// Track what type of message is currently being transmitted
typedef enum {
    SL_WSNCP_TX_TYPE_NONE = 0,
    SL_WSNCP_TX_TYPE_CNF,    // Confirmation message
    SL_WSNCP_TX_TYPE_IND     // Indication message
} sl_wsncp_tx_type_t;

static sl_wsncp_tx_type_t current_tx_type = SL_WSNCP_TX_TYPE_NONE;


// Forward declarations
static SL_NORETURN void sl_wsncp_task(void *argument);
static void sl_wsncp_req_handler(void);
static void sl_wsncp_ind_handler(void);

// Task flag management helpers
static inline void sl_wsncp_signal_request_ready(void)
{
  EFM_ASSERT((osEventFlagsSet(sl_wsncp_task_flags, SL_WSNCP_TASK_FLAG_REQ_READY) & CMSIS_RTOS_ERROR_MASK) == 0);
}

static inline void sl_wsncp_signal_indication_ready(void)
{
  EFM_ASSERT((osEventFlagsSet(sl_wsncp_task_flags, SL_WSNCP_TASK_FLAG_IND_READY) & CMSIS_RTOS_ERROR_MASK) == 0);
}

static inline void sl_wsncp_signal_indication_done(void)
{
  EFM_ASSERT((osEventFlagsSet(sl_wsncp_task_flags, SL_WSNCP_TASK_FLAG_IND_DONE) & CMSIS_RTOS_ERROR_MASK) == 0);
}

static inline void sl_wsncp_set_busy(void)
{
  EFM_ASSERT((osEventFlagsWait(sl_wsncp_task_flags,
                           SL_WSNCP_TASK_FLAG_COMM_READY,
                           osFlagsWaitAny,
                           osWaitForever) & CMSIS_RTOS_ERROR_MASK) == 0);
}

static inline void sl_wsncp_clr_busy(void)
{
  EFM_ASSERT((osEventFlagsSet(sl_wsncp_task_flags,
                              SL_WSNCP_TASK_FLAG_COMM_READY) & CMSIS_RTOS_ERROR_MASK) == 0);
}

// Buffer completion callbacks
static void sl_wsncp_req_complete_callback(void)
{
  sl_wsncp_signal_request_ready();
}

static void sl_wsncp_ind_complete_callback(void)
{
  sl_wsncp_signal_indication_ready();
}

/**
 * @brief Transmit data with proper buffer management and safety checks
 *
 * This function ensures that:
 * - Only one transmission can be active at a time
 * - Buffers are properly managed (cleared after transmission completion)
 * - All parameters are validated before transmission
 *
 * @param len Length of data to transmit
 * @param data Pointer to data to transmit
 * @param type String description for logging (e.g., "CNF", "IND")
 * @param id Message ID for logging
 * @param tx_type Type of transmission (CNF or IND) for proper buffer management
 */
static void sl_wsncp_transmit(uint16_t len, void *data, const char *type, uint32_t id, sl_wsncp_tx_type_t tx_type)
{
  uint32_t timediff;

  // Parameter validation
  if (data == NULL) {
    sl_wisun_trace_error("Data pointer is NULL for %s", type);
    return;
  }

  if (len == 0) {
    sl_wisun_trace_error("Length is zero for %s", type);
    return;
  }

  if (tx_type == SL_WSNCP_TX_TYPE_NONE) {
    sl_wisun_trace_error("Invalid transmission type for %s", type);
    return;
  }

  // Check if we're already transmitting
  if (current_tx_type != SL_WSNCP_TX_TYPE_NONE) {
    sl_wisun_trace_error("Already transmitting %s, current type: %d, new type: %d",
                        type, current_tx_type, tx_type);
    return;
  }

  // Set the current transmission type
  current_tx_type = tx_type;

  // Transmit the data
  sl_status_t status = sl_wsncp_interface_transmit(len, data);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Failed to transmit %s status: %u", type, status);
    current_tx_type = SL_WSNCP_TX_TYPE_NONE; // Reset on failure
    return;
  }

  // Track timing
  ncp_timestamp_tx = sl_sleeptimer_get_tick_count();
  sl_wsncp_set_busy();

  // Calculate and log timing
  timediff = sl_sleeptimer_get_tick_count() - ncp_timestamp_tx;
  sl_wisun_trace_debug("%s: id = %d transmitted in %"PRIu32"ms", type, id, sl_sleeptimer_tick_to_ms(timediff));
}

// -----------------------------------------------------------------------------
// Functions executed in IRQ context
// Transport layer callbacks - these are called directly by transport implementations
// Strong implementations that override the weak declarations in transport.h

void sl_wsncp_on_transmit_complete_cb(sl_status_t status)
{

  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Transmission failed, status: %d", status);
  }

  // Clear the appropriate buffer based on what was transmitted
  switch (current_tx_type) {
    case SL_WSNCP_TX_TYPE_CNF:
      sl_wsncp_buffer_clear(&sl_wsncp_cnf);
      break;

    case SL_WSNCP_TX_TYPE_IND:
      sl_wsncp_buffer_clear(&sl_wsncp_ind);
      sl_wsncp_signal_indication_done(); // Signal indication processing complete
      break;

    case SL_WSNCP_TX_TYPE_NONE:
    default:
      // Unexpected transmission type - no trace in IRQ context
      break;
  }

  // Reset transmission type
  current_tx_type = SL_WSNCP_TX_TYPE_NONE;

  // Transmission complete, let the task proceed
  sl_wsncp_clr_busy();
}

void sl_wsncp_on_receive_cb(sl_status_t status,
                            uint16_t len,
                            void* data)
{
  (void)status;

  sl_wsncp_buffer_add_data(&sl_wsncp_req, len, data, sl_wsncp_req_complete_callback);
}

int sl_wsncp_on_timeout_cb(uint32_t timeout_count,
                           uint16_t len,
                           void *data)
{
  const sl_wisun_msg_header_t *hdr;

  if (timeout_count >= SL_WSNCP_MAX_TIMEOUT_COUNT) {
    // Failsafe to stop RX if nothing has happened for a long time
    return 1;
  }

  hdr = (const sl_wisun_msg_header_t *)data;
  if ((len >= sizeof(sl_wisun_msg_header_t)) &&
      (len >= hdr->length)) {
    // We have enough data, RX can be stopped
    return 1;
  }

  // Not enough data
  return 0;
}

// -----------------------------------------------------------------------------
// Functions executed in application context

void sl_wsncp_task_init(void)
{
  // Initialize the interface layer
  sl_wsncp_interface_init();

  // Initialize message buffers with error checking
  sl_status_t status;

  status = sl_wsncp_buffer_init(&sl_wsncp_req);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Failed to initialize request buffer: %d", status);
    return;
  }

  status = sl_wsncp_buffer_init(&sl_wsncp_cnf);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Failed to initialize confirmation buffer: %d", status);
    return;
  }

  status = sl_wsncp_buffer_init(&sl_wsncp_ind);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Failed to initialize indication buffer: %d", status);
    return;
  }

  const osEventFlagsAttr_t sl_wsncp_task_flags_attr = {
    "Wi-SUN NCP Task Flags",
    0,
    NULL,
    0
  };

  sl_wsncp_task_flags = osEventFlagsNew(&sl_wsncp_task_flags_attr);
  EFM_ASSERT(sl_wsncp_task_flags != NULL);

  osThreadAttr_t sl_wsncp_task_attribute = {
    "Wi-SUN NCP Task",
    osThreadDetached,
    NULL,
    0,
    NULL,
    (SL_WSNCP_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u,
    SL_WSNCP_TASK_PRIORITY,
    0,
    0
    };

  sl_wsncp_task_id = osThreadNew(&sl_wsncp_task,
                                 NULL,
                                 &sl_wsncp_task_attribute);
  EFM_ASSERT(sl_wsncp_task_id != 0);

}

// -----------------------------------------------------------------------------
// Functions executed in service context

void sl_wisun_on_event(sl_wisun_evt_t *evt)
{
  if (evt == NULL) {
    sl_wisun_trace_error("Event pointer is NULL");
    return;
  }
  // Wait for any ongoing indication processing to complete before adding new data
  uint32_t flags = osEventFlagsWait(sl_wsncp_task_flags,
                                   SL_WSNCP_TASK_FLAG_IND_DONE,
                                   osFlagsWaitAny,
                                   osWaitForever);
  EFM_ASSERT((flags & CMSIS_RTOS_ERROR_MASK) == 0);

  // Add event data to the indication buffer
  sl_status_t status = sl_wsncp_buffer_add_data(&sl_wsncp_ind, evt->header.length, (const uint8_t*)evt, sl_wsncp_ind_complete_callback);

  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Failed to add indication data, status: %d", status);
  }
}

// -----------------------------------------------------------------------------
// Functions executed in the task context

static void sl_wsncp_req_handler(void)
{
  // Check if request buffer has a complete message
  if(sl_wsncp_buffer_get_state(&sl_wsncp_req) != SL_WSNCP_BUFFER_STATE_COMPLETE) {
    sl_wisun_trace_error("No complete request available, state: %d",
                         sl_wsncp_buffer_get_state(&sl_wsncp_req));
    return;
  }

  // Check if confirmation buffer is available (empty)
  if(sl_wsncp_buffer_get_state(&sl_wsncp_cnf) != SL_WSNCP_BUFFER_STATE_EMPTY) {
    sl_wisun_trace_error("Confirmation buffer not available, state: %d",
                         sl_wsncp_buffer_get_state(&sl_wsncp_cnf));
    return;
  }

  // Get request header for logging
  const sl_wisun_msg_header_t *req_header = (const sl_wisun_msg_header_t *)sl_wsncp_req.data;
  sl_wisun_trace_debug("REQ: id = %d len = %u", req_header->id, sl_wsncp_req.length);

  // Send request using the public API
  sl_status_t status = sl_wisun_send_request(sl_wsncp_req.data, sl_wsncp_req.length,
                                             sl_wsncp_cnf.data, SL_WSNCP_BUFFER_SIZE);
  if (status != SL_STATUS_OK) {
    sl_wisun_trace_error("Failed to send message request: %d", status);
    sl_wsncp_buffer_clear(&sl_wsncp_req);
    return;
  }

  // Get confirmation header for logging
  const sl_wisun_msg_header_t *cnf_header = (const sl_wisun_msg_header_t *)sl_wsncp_cnf.data;
  sl_wsncp_cnf.length = cnf_header->length;

  sl_wisun_trace_debug("CNF: id = %d len = %u", cnf_header->id, sl_wsncp_cnf.length);

  // Transmit the response (confirmation buffer will be cleared in transmit complete callback)
  sl_wsncp_transmit(sl_wsncp_cnf.length, sl_wsncp_cnf.data, "CNF", cnf_header->id, SL_WSNCP_TX_TYPE_CNF);

  // Clear request buffer for next message
  sl_wsncp_buffer_clear(&sl_wsncp_req);
}

static void sl_wsncp_ind_handler(void)
{
  // Check if indication buffer has a complete message
  if(sl_wsncp_buffer_get_state(&sl_wsncp_ind) != SL_WSNCP_BUFFER_STATE_COMPLETE) {
    sl_wisun_trace_error("No complete indication available, state: %d",
                         sl_wsncp_buffer_get_state(&sl_wsncp_ind));
    return;
  }

  // Get indication header
  const sl_wisun_msg_header_t *ind_header = (const sl_wisun_msg_header_t *)sl_wsncp_ind.data;
  sl_wisun_trace_debug("IND: id = %d len = %u", ind_header->id, sl_wsncp_ind.length);

  // Transmit the indication (indication buffer will be cleared in transmit complete callback)
  sl_wsncp_transmit(sl_wsncp_ind.length, sl_wsncp_ind.data, "IND", ind_header->id, SL_WSNCP_TX_TYPE_IND);

}

static void sl_wsncp_task(void *argument)
{
  uint32_t flags;
  (void)argument;

  // Clear buffers initially
  sl_wsncp_buffer_clear(&sl_wsncp_req);
  sl_wsncp_buffer_clear(&sl_wsncp_ind);

  // Allow first indication
  sl_wsncp_signal_indication_done();

  while (1) {
    // Wait for a request or indication
    flags = osEventFlagsWait(sl_wsncp_task_flags,
                             SL_WSNCP_TASK_FLAG_REQ_READY + SL_WSNCP_TASK_FLAG_IND_READY,
                             osFlagsWaitAny,
                             osWaitForever);
    EFM_ASSERT((flags & CMSIS_RTOS_ERROR_MASK) == 0);

    // Handle request
    if (flags & SL_WSNCP_TASK_FLAG_REQ_READY) {
      sl_wsncp_req_handler();
    }

    // Handle indication
    if (flags & SL_WSNCP_TASK_FLAG_IND_READY) {
      sl_wsncp_ind_handler();
    }
  }
}
