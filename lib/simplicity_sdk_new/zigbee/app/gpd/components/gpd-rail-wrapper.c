/***************************************************************************//**
 * @file
 * @brief Routines built on RAIL functions used by the GPD.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "gpd-components-common.h"

static sl_rail_handle_t railHandle = NULL;
static bool     rfReady = false;

// This buffer is required to be RAIL_FIFO_ALIGNMENT aligned,
// which is 4 bytes for Series 2
static union {
  // Used to align this buffer as needed
  SL_RAIL_DECLARE_FIFO_BUFFER(align, GP_FIFO_SIZE);
  uint8_t fifo[GP_FIFO_SIZE];
} txFifoAligned;
#define railTxFifo (txFifoAligned.fifo)

void sl_rail_util_on_event(sl_rail_handle_t railHandle,
                           sl_rail_events_t events)
{
  (void)railHandle;

  if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
    CORE_irqState_t c = CORE_EnterCritical();
    uint16_t bytesAvailable = sl_rail_get_rx_fifo_bytes_available(railHandle);
    uint16_t rxReceived = sl_rail_read_rx_fifo(railHandle, sl_zigbee_gpd_get_rx_mpdu(), bytesAvailable);
    sl_zigbee_gpd_incoming_message_handler(sl_zigbee_gpd_get_rx_mpdu(), rxReceived);
    CORE_ExitCritical(c);
  }
}

void sl_rail_util_on_rf_ready(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  rfReady = true;
}

void sl_zigbee_gpd_rail_provide_rail_handle(sl_rail_handle_t handle)
{
  railHandle = handle;
}

void sl_zigbee_gpd_rail_start_rx_wrapper(uint8_t channel)
{
  sl_rail_start_rx(railHandle, channel, NULL);
}

void sl_zigbee_gpd_rail_idle_wrapper(void)
{
  sl_rail_idle(railHandle, SL_RAIL_IDLE, true);
}

void sl_zigbee_gpd_rail_start_tx_wrapper(bool skipCca,
                                         uint16_t channel)
{
  sl_rail_status_t status;
  if (skipCca) {
    status = sl_rail_start_tx(railHandle,
                              channel,
                              SL_RAIL_TX_OPTIONS_DEFAULT,
                              NULL);
  } else {
    sl_rail_csma_config_t txCsmaOptions = SL_RAIL_CSMA_CONFIG_802_15_4_2003_2P4_GHZ_OQPSK_CSMA;
    status = sl_rail_start_cca_csma_tx(railHandle,
                                       channel,
                                       SL_RAIL_TX_OPTIONS_DEFAULT,
                                       &txCsmaOptions,
                                       NULL);
  }
  if (status != 0) {
    while (true) ; // Error - Trap it
  }
}

void sl_zigbee_gpd_rail_write_tx_fifo_wrapper(const uint8_t *dataPtr,
                                              uint16_t writeLength)
{
  uint16_t writtenLength = sl_rail_write_tx_fifo(railHandle,
                                                 dataPtr,
                                                 writeLength,
                                                 true);
  if (writtenLength != writeLength) {
    while (true) ;
  }
}

uint16_t sl_zigbee_gpd_rail_get_radio_entropy_wrapper(uint8_t *dataPtr, uint16_t dataLength)
{
  return sl_rail_get_radio_entropy(railHandle, dataPtr, dataLength);
}

void sl_zigbee_gpd_radio_init(void)
{
  // Set TX FIFO, and verify that the size is correct
  sl_rail_status_t status = sl_rail_set_tx_fifo(railHandle, (sl_rail_fifo_buffer_align_t*)railTxFifo, GP_FIFO_SIZE, 0, 0);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    while (true) ;
  }

  // Write the PAN ID as 0xFFFF
  if (sl_rail_ieee802154_set_pan_id(railHandle, 0xFFFF, 0x00) != SL_RAIL_STATUS_NO_ERROR) {
    while (true) ;
  }
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID) && (SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID == SL_ZIGBEE_GPD_APP_ID_IEEE_ID)
  uint8_t gpdIeee[] = SL_ZIGBEE_AF_PLUGIN_APPS_IEEE;
  uint8_t gpdEndpoint = SL_ZIGBEE_AF_PLUGIN_APPS_GPD_ENDPOINT;
  sl_zigbee_gpd_af_plugin_get_gpd_ieee_cb(gpdIeee,
                                          &gpdEndpoint);
  // Set Long address.
  if (sl_rail_ieee802154_set_long_address(railHandle, gpdIeee, 0x00) != SL_RAIL_STATUS_NO_ERROR) {
    while (true) ;
  }
#endif

  while (!rfReady) ;
}
