/***************************************************************************/ /**
 * @file
 * @brief CPC SDIO Driver implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "rsi_debug.h"
#include "rsi_pll.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_table_si91x.h"
#include "sl_cpc_config.h"
#include "sl_cpc_drv_secondary_sdio_config.h"
#include "sl_si91x_peripheral_sdio_secondary.h"
#include "sl_si91x_sdio_secondary_drv_config.h"
#include "sli_cpc.h"
#include "sli_cpc_crc.h"
#include "sli_cpc_debug.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_hdlc.h"
#include "sli_cpc_system_common.h"
#include "sli_cpc_assert.h"
/*******************************************************************************
 ***************************  DEFINES ******************************************
 ******************************************************************************/
#define DMA_MAX_XFER_LEN         4096
#define CPC_TASK_STACK_SIZE      4096
#if defined(SL_CATALOG_KERNEL_PRESENT)
#define EVENT_CPC_TX_FLUSH       0x01
#define EVENT_CPC_IDLE           0x02
#define EVENT_CPC_RX_IRQ_PENDING 0x03
#define EVENT_CPC_RX_FLUSH       0x04
#endif

#if (SLI_CPC_RX_DATA_MAX_LENGTH > DMA_MAX_XFER_LEN)
#error "This driver only supports 2560 data transaction SLI_CPC_RX_DATA_MAX_LENGTH should be set not more than 2560"
#endif
/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/
enum { HEADER_CORRUPTED, HEADER_NULL, HEADER_VALID } received_header_situation;

typedef enum {
  SDIO_91X_CPC_IDLE = 0,
  SDIO_91X_CPC_TX_IN_PROGRESS,
  SDIO_91X_CPC_TX_FLUSH,
  SDIO_91X_CPC_RX_IRQ_PENDING,
  SDIO_91X_CPC_RX_IN_PROGRESS,
  SDIO_91X_CPC_RX_FLUSH,
  SDIO_91X_CPC_END // Add any new enum's on top of this
} sdio_91x_cpc_state_t;

enum {
  SDIO_91X_IRQ_IDLE = 0,
  SDIO_91X_IRQ_TX_IN_PROGRESS,
  SDIO_91X_IRQ_TX_DONE,
  SDIO_91X_IRQ_RX_INTERRUPT_PENDING,
  SDIO_91X_IRQ_RX_IN_PROGRESS,
  SDIO_91X_IRQ_RX_DONE,
  SDIO_91X_IRQ_END // Add any new enum's on top of this
} sdio_91x_irq_state_t;

/*******************************************************************************
 ******************************   VARIABLES   **********************************
 ******************************************************************************/
// List of "sli_buf_entry_t" which have an EMPTY "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that are available for receiving a frame. When arming a reception,
// the driver picks the first one and configures the DMA to write the data into its
// attached buffer.
static sl_slist_node_t *rx_free_list_head;

// List of "sli_buf_entry_t" which have a "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that have fully received frame written in the attached
// "sl_cpc_buffer_handle_t". They are waiting for the core to pick them up,
// detach the filled buffer and being put in the "rx_free_no_buf_list_head" list.
static sl_slist_node_t *rx_pending_list_head;

// List of "sli_buf_entry_t" which have a FILLED "sl_cpc_buffer_handle_t" attached to them.
// Those are entries that are waiting for its buffer to be sent on the wire.
static sl_slist_node_t *tx_submitted_list_head;

static uint16_t tx_buf_available_count = SL_CPC_DRV_SDIO_TX_QUEUE_SIZE;

// When not null, points to the first entry in "rx_free_list_head" when the driver
// Set itself up to receive a new frame. During the transaction, it is in this entry's
// buffer that the DMA writes the data.
static sl_cpc_buffer_handle_t *currently_receiving_rx_entry = NULL;

// When not null, points to the buffer handle currently being transmitted and configured in the active
// TX DMA chain.
static sl_cpc_buffer_handle_t *currently_transmiting_tx_entry = NULL;

static bool pending_late_header = false;

static volatile uint32_t tx_frame_complete                 = 0;
static uint8_t header_buffer[SLI_CPC_HDLC_HEADER_RAW_SIZE] = { 0 };

static volatile uint16_t tx_payload_len  = 0;
static volatile uint16_t security_length = 0;
static volatile sdio_91x_cpc_state_t cpc_tx_state            = SDIO_91X_CPC_IDLE;
static volatile sdio_91x_cpc_state_t cpc_rx_state            = SDIO_91X_CPC_IDLE;
#if defined(SL_CATALOG_KERNEL_PRESENT)
static osEventFlagsId_t cpc_bus_events = NULL;
#endif

uint8_t xfer_buffer[DMA_MAX_XFER_LEN];
/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void flush_rx(void);
static void flush_tx(void);
static void end_of_header_xfer(void);
static void end_of_payload_xfer(void);
static void prime_dma_for_transmission(void);
static void prime_dma_for_reception(size_t payload_size);
static sl_status_t sdio_drv_hw_init(sli_cpc_drv_t *drv);
static sl_status_t sdio_drv_init(sli_cpc_drv_t *drv, sli_cpc_instance_t *inst);
static sl_status_t sdio_drv_get_capabilities(sli_cpc_drv_t *drv, sli_cpc_drv_capabilities_t *capabilities);
static sl_status_t sdio_drv_read_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t **buffer_handle);
static sl_status_t sdio_drv_transmit_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t *buffer_handle);
static bool sdio_drv_is_transmit_ready(sli_cpc_drv_t *drv);
static uint32_t sdio_drv_get_bus_bitrate(sli_cpc_drv_t *drv);
static uint32_t sdio_drv_get_bus_max_bitrate(sli_cpc_drv_t *drv);
static void sdio_drv_on_rx_buffer_handle_free(sli_cpc_drv_t *drv);
static void sdio_drv_on_rx_buffer_free(sli_cpc_drv_t *drv);
static sl_status_t sdio_drv_start_rx(sli_cpc_drv_t *drv);

static sli_cpc_instance_t *sdio_driver_inst;

static uint8_t rx_data_buffer[DMA_MAX_XFER_LEN];

volatile bool cpc_start_tx = 0;

#if defined(SL_CATALOG_KERNEL_PRESENT)
const osThreadAttr_t cpc_app_thread_attributes = {
  .name       = "cpc_sdio_driver",       // Name of thread
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = CPC_TASK_STACK_SIZE,                  // Stack size of cpc_sdio_driver task
  .priority   = osPriorityAboveNormal2, // Priority of cpc_sdio_driver task
  .tz_module  = 0,
  .reserved   = 0,
};
#endif

sli_cpc_drv_t sdio_driver = { .ops = {
                                .hw_init          = sdio_drv_hw_init,
                                .init             = sdio_drv_init,
                                .get_capabilities = sdio_drv_get_capabilities,
                                .start_rx         = sdio_drv_start_rx,
                                .read                     = sdio_drv_read_data,
                                .write                    = sdio_drv_transmit_data,
                                .is_transmit_ready        = sdio_drv_is_transmit_ready,
                                .get_bus_bitrate          = sdio_drv_get_bus_bitrate,
                                .get_bus_max_bitrate      = sdio_drv_get_bus_max_bitrate,
                                .on_rx_buffer_handle_free = sdio_drv_on_rx_buffer_handle_free,
                                .on_rx_buffer_free        = sdio_drv_on_rx_buffer_free,
                              } };

/***************************************************************************/ /**
 * Write bytes to SDIO.
 ******************************************************************************/
static sl_status_t sdio_drv_transmit_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t *buffer_handle)
{
  (void)drv;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  if (tx_buf_available_count == 0) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_NOT_READY;
  }
  tx_buf_available_count--;
  sli_cpc_push_back_driver_buffer_handle(&tx_submitted_list_head, buffer_handle);

  prime_dma_for_transmission();

  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Read bytes from SDIO.
 ******************************************************************************/
static sl_status_t sdio_drv_read_data(sli_cpc_drv_t *drv, sl_cpc_buffer_handle_t **buffer_handle)
{
  (void)drv;
  sl_status_t status;
  MCU_DECLARE_IRQ_STATE;

  sl_cpc_buffer_handle_t *new_buffer_handle;

  MCU_ENTER_ATOMIC();
  sl_cpc_buffer_handle_t *pending_buffer_handle = sli_cpc_pop_driver_buffer_handle(&rx_pending_list_head);
  if (pending_buffer_handle == NULL) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_EMPTY;
  }
  MCU_EXIT_ATOMIC();

  *buffer_handle = pending_buffer_handle;

  MCU_ENTER_ATOMIC();
  status = sli_cpc_get_buffer_handle_for_rx(sdio_driver_inst, &new_buffer_handle, true);
  if (status == SL_STATUS_OK) {
    sli_cpc_push_driver_buffer_handle(&rx_free_list_head, new_buffer_handle);
  }
  MCU_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Start reception
 ******************************************************************************/
static sl_status_t sdio_drv_start_rx(sli_cpc_drv_t *drv)
{
  (void)drv;
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Gets CPC driver capabilities.
 ******************************************************************************/
static sl_status_t sdio_drv_get_capabilities(sli_cpc_drv_t *drv, sli_cpc_drv_capabilities_t *capabilities)
{
  (void)drv;
  if (capabilities == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  *capabilities                        = (sli_cpc_drv_capabilities_t){ 0 };
  capabilities->preprocess_hdlc_header = true;
  capabilities->uart_flowcontrol       = true;

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Initialize only the SDIO peripheral On the secondary (this) side.
 ******************************************************************************/
static sl_status_t sdio_drv_hw_init(sli_cpc_drv_t *drv)
{
  (void)drv;
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * gpdma callback handler. this updates the state of Tx and Rx
 ******************************************************************************/
static void sli_cpc_sdio_gpdma_drv_callback_slave(uint32_t event)
{
  switch (event) {
    case SDIO_91X_IRQ_IDLE:
    case SDIO_91X_IRQ_RX_IN_PROGRESS:
    case SDIO_91X_IRQ_TX_IN_PROGRESS:
      break;
    case SDIO_91X_IRQ_RX_DONE: {
      cpc_rx_state = SDIO_91X_CPC_RX_FLUSH;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      osEventFlagsSet(cpc_bus_events, EVENT_CPC_RX_FLUSH);
#endif
    } break;
    case SDIO_91X_IRQ_TX_DONE: {
      //! Flush the TX packet to CPC deamon for processing
      cpc_tx_state = SDIO_91X_CPC_TX_FLUSH;
#if defined(SL_CATALOG_KERNEL_PRESENT)
      osEventFlagsSet(cpc_bus_events, EVENT_CPC_TX_FLUSH);
#endif
    } break;
  }
}

/***************************************************************************/ /**
 * SDIO IRQ handler
 ******************************************************************************/
void sli_cpc_sdio_irq_callback(uint8_t events)
{
  (void)events;
  //! Set the cpc rx state to in progress
  cpc_rx_state   = SDIO_91X_CPC_RX_IN_PROGRESS;
  sl_si91x_sdio_secondary_receive(&rx_data_buffer[0]);
}

/***************************************************************************/ /**
 * gpdma transmission done callback handler
 ******************************************************************************/
static void sli_cpc_sdio_gpdma_tx_done_callback()
{
  //! Set the CPC state Machine to RX done callback
  sli_cpc_sdio_gpdma_drv_callback_slave(SDIO_91X_IRQ_TX_DONE);
}

/***************************************************************************/ /**
 * gpdma receive done callback handler
 ******************************************************************************/
static void sli_cpc_sdio_gpdma_rx_done_callback()
{
  //! Set the CPC state Machine to RX done callback
  sli_cpc_sdio_gpdma_drv_callback_slave(SDIO_91X_IRQ_RX_DONE);
}

/***************************************************************************/ /**
 * gpdma callback handler
 ******************************************************************************/
static void gdpma_callback(uint8_t dma_ch)
{
  if (dma_ch == GPDMA_CHNL1) {
    sli_cpc_sdio_gpdma_rx_done_callback();
  } else if (dma_ch == GPDMA_CHNL0) {
    sli_cpc_sdio_gpdma_tx_done_callback();
  }
}

/***************************************************************************/ /**
 * Notifies the core when a frame has been received on the wire.
 ******************************************************************************/
static void flush_rx(void)
{
  if (currently_receiving_rx_entry && received_header_situation == HEADER_VALID) {
    sli_cpc_push_back_driver_buffer_handle(&rx_pending_list_head, currently_receiving_rx_entry);

    currently_receiving_rx_entry = NULL;

    sli_cpc_notify_rx_data_from_drv(sdio_driver_inst);
  }
}

/***************************************************************************/ /**
 * Reconfigure only what needs to be reconfigured in the RX DMA descriptor chain
 * and start the RX DMA channel
 ******************************************************************************/
static void prime_dma_for_reception(size_t payload_size)
{
  // Since we received a valid header, we are going to need a rx_entry to place it into
  if (currently_receiving_rx_entry == NULL) {
    currently_receiving_rx_entry = sli_cpc_pop_driver_buffer_handle(&rx_free_list_head);
  }

  if (currently_receiving_rx_entry == NULL) {
    // running out of buffer. We will drop this frame.
    // Configure the reception chain to skip the payload.
    // Having currently_receiving_rx_entry == NULL when flushing_rx will drop this frame
    DEBUGOUT("\r\n Out of CPC RX buffers\r\n");
    SLI_CPC_ASSERT(0);
  }

  // Copy the valid header from the static header buffer into the buffer_handle
  memcpy(currently_receiving_rx_entry->hdlc_header, &header_buffer[0], SLI_CPC_HDLC_HEADER_RAW_SIZE);

  if (payload_size != 0) {
    memcpy(currently_receiving_rx_entry->data, &rx_data_buffer[SLI_CPC_HDLC_HEADER_RAW_SIZE], payload_size);
  } else {
  }
}

/***************************************************************************/ /**
 * Process received header buffer
 ******************************************************************************/
static void end_of_header_xfer(void)
{
  size_t rx_payload_length                        = 0;
  uint8_t null_buff[SLI_CPC_HDLC_HEADER_RAW_SIZE] = { 0 };

  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();

  memcpy(header_buffer, &rx_data_buffer, SLI_CPC_HDLC_HEADER_RAW_SIZE);
  // Check for rx header validity
  if (memcmp(header_buffer, null_buff, SLI_CPC_HDLC_HEADER_RAW_SIZE) == 0) {
    received_header_situation = HEADER_NULL;
  } else {
    bool valid_header = sli_cpc_validate_crc_sw((void *)&header_buffer[0],
                                                SLI_CPC_HDLC_HEADER_SIZE,
                                                sli_cpc_hdlc_get_hcs((void *)&header_buffer[0]));
    if (valid_header) {
      received_header_situation = HEADER_VALID;
      rx_payload_length         = sli_cpc_hdlc_get_length((void *)&header_buffer[0]);
    } else {
      received_header_situation = HEADER_CORRUPTED;
    }
  }

  if (received_header_situation == HEADER_VALID) {
    prime_dma_for_reception(rx_payload_length);
  }

  MCU_EXIT_ATOMIC();
}

/***************************************************************************/ /**
 * Notifies the core when a frame has been sent on the wire.
 ******************************************************************************/
static void end_of_payload_xfer(void)
{
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();

  end_of_header_xfer();

  // Check if we had a late header transmission situation
  if (currently_transmiting_tx_entry) {
    // At this point, a header have been exchanged and we realize a TX entry is registered for transmission
    // We need to know is this "currently_transmiting_tx_entry" had the chance to have its header clocked in the header
    // exchange that just happened
    if (tx_frame_complete == 0) {
      // This "currently_transmiting_tx_entry" has arrived lated. Yes it is at this point the current active TX frame, but
      // at the moment the primary started clocking the header, its header did't go through. Mark this current
      // TX entry transmission as a "pending_late_header" so that we don't try to flush this "currently_transmiting_tx_entry"
      // as having been sent on the wire.
      pending_late_header = true;
    }
  }

  // Reset that variable which is set to 1 by the TX DMA chain
  tx_frame_complete = 0;

  flush_rx();
  flush_tx();

  // Check if we have something to send
  if (tx_submitted_list_head) {
    if (pending_late_header) {
      pending_late_header = false;
    } else {
      // Prepare for tx
      prime_dma_for_transmission();
    }
  }

  MCU_EXIT_ATOMIC();
}

/***************************************************************************/ /**
 * Reconfigure only what needs to be reconfigured in the TX DMA descriptor chain
 * and start the TX DMA channel.
 *
 * We assume the TX DMA channel is not running and we are in interrupt context
 ******************************************************************************/
static void prime_dma_for_transmission(void)
{
  uint8_t fcs[2]       = { 0 };
  uint8_t no_of_blocks = 0;

  if (currently_transmiting_tx_entry) {
    // There is already a frame programmed to be sent. Do nothing now, when the
    // frame currently programmed in the TX DMA chain gets tx_flushed(), the
    // frame for which this function was called will be cocked in.
    return;
  }

  currently_transmiting_tx_entry = SL_SLIST_ENTRY(tx_submitted_list_head, sl_cpc_buffer_handle_t, driver_node);

  tx_payload_len  = 0;
  security_length = 0;
  // Bug if this function is called but there is nothing in the submitted list
  SLI_CPC_ASSERT(currently_transmiting_tx_entry != NULL);

  memcpy(&xfer_buffer[0], currently_transmiting_tx_entry->hdlc_header, SLI_CPC_HDLC_HEADER_RAW_SIZE);
  tx_payload_len += SLI_CPC_HDLC_HEADER_RAW_SIZE;
  if (currently_transmiting_tx_entry->data_length == 0) {
    //  No payload, we are done so start the transfer.
    goto start_transfer;
  }

  // Copy the payload
  memcpy(&xfer_buffer[SLI_CPC_HDLC_HEADER_RAW_SIZE],
         currently_transmiting_tx_entry->data,
         currently_transmiting_tx_entry->data_length);
  tx_payload_len += currently_transmiting_tx_entry->data_length;
#if (SL_CPC_ENDPOINT_SECURITY_ENABLED == 1)
  if (currently_transmiting_tx_entry->security_tag) {
    // Copy the tag
    memcpy(&xfer_buffer[SLI_CPC_HDLC_HEADER_RAW_SIZE + (currently_transmiting_tx_entry->data_length)],
           currently_transmiting_tx_entry->security_tag,
           SLI_SECURITY_TAG_LENGTH_BYTES);
    security_length += SLI_SECURITY_TAG_LENGTH_BYTES;
  } else {
    security_length = 0;
  }
  tx_payload_len += security_length;
#endif

  fcs[0] = (uint8_t)currently_transmiting_tx_entry->fcs;
  fcs[1] = (uint8_t)(currently_transmiting_tx_entry->fcs >> 8);
  // Finally load the checksum
  memcpy((&xfer_buffer[currently_transmiting_tx_entry->data_length + security_length + SLI_CPC_HDLC_HEADER_RAW_SIZE]),
         fcs,
         sizeof(fcs));
  tx_payload_len += sizeof(fcs);

  start_transfer:
  // we are done so start the transfer.
  no_of_blocks = (((tx_payload_len + (255)) & ~(255)) >> 8);

  cpc_tx_state = SDIO_91X_CPC_TX_IN_PROGRESS;
  sl_si91x_sdio_secondary_send(no_of_blocks, xfer_buffer);
}

/***************************************************************************/ /**
 * Notifies the core when a frame has been sent on the wire.
 ******************************************************************************/
static void flush_tx(void)
{
  if (currently_transmiting_tx_entry == NULL) {
    // Nothing to notify the core about if there is no current TX entry
    return;
  }

  if (pending_late_header) {
    // There is a TX entry, but we realized the header couldn't be sent in time
    return;
  }

  // Remove the first entry from the TX submitted list.
  sl_cpc_buffer_handle_t *buffer_handle = sli_cpc_pop_driver_buffer_handle(&tx_submitted_list_head);

  // Paranoia. The first entry in the TX submitted list NEEDS to be the currently_transmiting_tx_entry
  SLI_CPC_ASSERT(buffer_handle == currently_transmiting_tx_entry);

  // Notify the core that this entry has been sent on the wire. It will detach its buffer
  sli_cpc_notify_tx_data_by_drv(buffer_handle);

  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();
  ++tx_buf_available_count;
  SLI_CPC_ASSERT(tx_buf_available_count <= SL_CPC_DRV_SDIO_TX_QUEUE_SIZE);
  MCU_EXIT_ATOMIC();

  // Important to set this back to NULL
  currently_transmiting_tx_entry = NULL;
}

/***************************************************************************/ /**
 * Handles the secondary tx and rx logic based on cpc_tx_rx_state
 * M4 core respectively.
 ******************************************************************************/
void sl_cpc_process_tx_rx(void)
{
  if (cpc_tx_state == SDIO_91X_CPC_TX_FLUSH) {
    flush_tx();
    cpc_tx_state = SDIO_91X_CPC_IDLE;
  }

  if (cpc_tx_state == SDIO_91X_CPC_IDLE) {
    // Check if we have something to send
    if (tx_submitted_list_head) {
      if (pending_late_header) {
        pending_late_header = false;
      } else {
        // Prepare for tx
        prime_dma_for_transmission();
      }
    }
  }

  if (cpc_rx_state == SDIO_91X_CPC_RX_IRQ_PENDING) {
  } else if (cpc_rx_state == SDIO_91X_CPC_RX_FLUSH) {
    if (rx_data_buffer[1] == 0xEE) {
      cpc_start_tx = 1;
    } else {
      // Rx is completed inform CPC core
      end_of_payload_xfer();
      // Enable the SDIO write interrupt
    }
    sl_si91x_sdio_secondary_set_interrupts(SL_SDIO_WR_INT_UNMSK);

    cpc_rx_state = SDIO_91X_CPC_IDLE;
  }
}

/***************************************************************************/ /**
 * Process the cpc Tx and Rx
 ******************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
void sl_si91x_cpc_app_task(void *arg)   // Change the function name to something more meaningful....
{
  (void)arg;
  uint32_t flags;
  while (1) {
    // Wait for START TEST or STOP TEST flags
    flags = osEventFlagsWait(cpc_bus_events,
                             EVENT_CPC_TX_FLUSH | EVENT_CPC_IDLE | EVENT_CPC_RX_IRQ_PENDING | EVENT_CPC_RX_FLUSH,
                             osFlagsWaitAny,
                             osWaitForever);
    SLI_CPC_ASSERT((flags & osFlagsError) == 0);

    sl_cpc_process_tx_rx();
  }
}
#endif

/***************************************************************************/ /**
 * Initialize the rest of the driver after the SDIO peripheral has been
 * initialized in sdio_drv_hw_init.
 ******************************************************************************/
static sl_status_t sdio_drv_init(sli_cpc_drv_t *drv, sli_cpc_instance_t *inst)
{
  (void)drv;
  int32_t status = 0;

  // Initialize the lists
  sl_slist_init(&rx_free_list_head);
  sl_slist_init(&rx_pending_list_head);
  sl_slist_init(&tx_submitted_list_head);

  sdio_driver_inst = inst;

  for (uint32_t buf_cnt = 0; buf_cnt < SL_CPC_DRV_SDIO_RX_QUEUE_SIZE; buf_cnt++) {
    sl_cpc_buffer_handle_t *buffer_handle;
    if (sli_cpc_get_buffer_handle_for_rx(sdio_driver_inst, &buffer_handle, true) != SL_STATUS_OK) {
      SLI_CPC_ASSERT(false);
      return SL_STATUS_ALLOCATION_FAILED;
    }
    sli_cpc_push_driver_buffer_handle(&rx_free_list_head, buffer_handle);
  }

  tx_buf_available_count = SL_CPC_DRV_SDIO_TX_QUEUE_SIZE;

  //! Initialize SDIO Slave
  status = sl_si91x_sdio_secondary_register_event_callback(sli_cpc_sdio_irq_callback, SL_SDIO_WR_INT_EN);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nSDIO Secondary callback function registration failed\r\n");
  }

  sl_si91x_sdio_secondary_gpdma_register_event_callback(gdpma_callback);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  cpc_bus_events = osEventFlagsNew(NULL);

  osThreadNew((osThreadFunc_t)sl_si91x_cpc_app_task, NULL, &cpc_app_thread_attributes);
#endif
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 * Checks if driver is ready to transmit.
 ******************************************************************************/
static bool sdio_drv_is_transmit_ready(sli_cpc_drv_t *drv)
{
  (void)drv;
  return true;
}

/***************************************************************************/ /**
 * Get currently configured bus bitrate
 ******************************************************************************/
static uint32_t sdio_drv_get_bus_bitrate(sli_cpc_drv_t *drv)
{
  (void)drv;
  return 0;
}

/***************************************************************************/ /**
 * Get maximum bus bitrate
 ******************************************************************************/
static uint32_t sdio_drv_get_bus_max_bitrate(sli_cpc_drv_t *drv)
{
  (void)drv;
  return 0;
}

/***************************************************************************/ /**
 * Notification when RX buffer becomes free.
 ******************************************************************************/
static void sdio_drv_on_rx_buffer_free(sli_cpc_drv_t *drv)
{
  (void)drv;
}

/***************************************************************************/ /**
 * Notification when RX buffer handle becomes free.
 ******************************************************************************/
static void sdio_drv_on_rx_buffer_handle_free(sli_cpc_drv_t *drv)
{
  (void)drv;
  sl_status_t status;
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  do {
    sl_cpc_buffer_handle_t *buffer_handle;

    status = sli_cpc_get_buffer_handle_for_rx(sdio_driver_inst, &buffer_handle, true);
    if (status == SL_STATUS_OK) {
      sli_cpc_push_driver_buffer_handle(&rx_free_list_head, buffer_handle);
    }
  } while (status == SL_STATUS_OK);
  MCU_EXIT_ATOMIC();
}

/***************************************************************************/ /**
 * Return a pointer to the UART driver instance.
 ******************************************************************************/
sli_cpc_drv_t *sli_cpc_drv_get_driver(void)
{
  return &sdio_driver;
}
