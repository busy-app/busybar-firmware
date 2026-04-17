/***************************************************************************//**
 * @file
 * @brief GPD commissioning functions.
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
#include "gpd-cb.h"

static uint8_t txMpdu[SL_ZIGBEE_GPD_TX_MPDU_SIZE];
static uint8_t rxMpdu[SL_ZIGBEE_GPD_RX_MPDU_SIZE];

uint8_t * sl_zigbee_gpd_get_rx_mpdu(void)
{
  return rxMpdu;
}
// Starts to receive for receiveWindowInUs duration after waiting for
// startDelayInUs. During the startDelayInUs the GPD can sleep to an appropriate
// and possible sleep mode to conserve energy.
static void gpdScheduledReceive(uint32_t startDelayInUs,
                                uint32_t receiveWindowInUs,
                                uint8_t channel,
                                bool sleepInDelay)
{
  // Start LE timer with startDelay
  // Put the micro to low power sleep with above wake up configured
  // Wake Up when start delay time expires
  // Keep Receiver On to receive the data for recieveWindow
  if (startDelayInUs) {
    sl_zigbee_gpd_load_le_timer(startDelayInUs);
    // If Enters EM0 instead or awaken by other things, made to wait by following
    // code until the LE Timer expires to provide the exact rxOffset before receive.
    // the callback will be responsible to put the micro in the sleep
    while (sl_zigbee_gpd_le_timer_running()) {
      if (sleepInDelay) {
        sl_zigbee_gpd_af_plugin_sleep_cb();
      }
    }
  }

  // Load the timer for the receive window
  sl_zigbee_gpd_load_le_timer(receiveWindowInUs);
  // Start the receiver
  sl_zigbee_gpd_rail_start_rx_wrapper(channel);

  //Code blocker for the entire time of receive window
  while (sl_zigbee_gpd_le_timer_running()) ;
}

int8_t sl_zigbee_af_gpdf_send(uint8_t frameType,
                              sl_zigbee_gpd_t_t * gpd,
                              uint8_t * payload,
                              uint8_t payloadLength,
                              uint8_t repeatNumber)
{
  // EMZIGBEE-11785 : Using CSMA in GPDF Send function is restricted.
  // The reasons is that the GPD being min energy devices, don't use CSMA in most of the designs,
  // rather the designs prefer to send out multiple packets using the same energy budget.
  if (!gpd->skipCca) {
    return FAILED;
  }
  // Check packet length
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID) && (SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID == SL_ZIGBEE_GPD_APP_ID_SRC_ID)
  payloadLength = SL_MIN(payloadLength, SL_ZIGBEE_GPD_SRC_ID_MAX_PAYLOAD_SIZE);
#elif defined(SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID) && (SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID == SL_ZIGBEE_GPD_APP_ID_IEEE_ID)
  payloadLength = SL_MIN(payloadLength, SL_ZIGBEE_GPD_IEEE_ID_MAX_PAYLOAD_SIZE);
#else
#error "Unsupported GPD Application Id"
#endif

  if (frameType == SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA
      || frameType == SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_MAINT) {
    // update Security frame counter before building packet
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ) && (SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ == SL_ZIGBEE_GPD_MAC_SEQ_INCREMENTAL)
    gpd->securityFrameCounter++;
    sl_zigbee_gpd_save_gpd_frame_counter_to_shadow(gpd->securityFrameCounter);
    // update NONCE TX & Rx
    sl_zigbee_gpd_set_fc_into_tx_nonce(gpd->securityFrameCounter);
    sl_zigbee_gpd_set_fc_into_rx_nonce(gpd->securityFrameCounter);

#elif defined(SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ) && (SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ == SL_ZIGBEE_GPD_MAC_SEQ_RANDOM)
    gpd->securityFrameCounter = (uint8_t)sl_zigbee_gpd_af_plugin_get_random_cb();
#else
#error "Undefined GPD MAC SEQ Mode"
#endif
  }
  uint8_t length = sl_zigbee_build_out_going_pdu(frameType,
                                                 txMpdu,
                                                 payload,
                                                 payloadLength,
                                                 gpd);
  // local variable
  uint8_t repeat = 0;
  do {
    sl_zigbee_gpd_rail_write_tx_fifo_wrapper(txMpdu, length);
    sl_zigbee_gpd_rail_idle_wrapper();
    uint32_t preTxRailTime = sl_rail_get_time(SL_RAIL_EFR32_HANDLE);
    //
    sl_zigbee_gpd_rail_start_tx_wrapper(gpd->skipCca, gpd->channel);
    sl_zigbee_gpd_rail_idle_wrapper();
    //
    if (gpd->rxAfterTx) {
      uint32_t txRailDurationUs = sl_rail_get_time(SL_RAIL_EFR32_HANDLE) - preTxRailTime;
      gpdScheduledReceive((((uint32_t)gpd->rxOffset * 1000) - txRailDurationUs),
                          (uint32_t)(gpd->minRxWindow) * 1000,
                          gpd->channel,
                          true);
      sl_zigbee_gpd_rail_idle_wrapper();
    }
    repeat++;
  } while (repeat < repeatNumber);

  return SUCCESS;
}
#ifndef USER_HAS_GPD_INCOMING_COMMAND_HANDLER
void sl_zigbee_gpd_incoming_command_handler(uint8_t *gpdCommandBuffer, uint8_t length)
{
  if (gpdCommandBuffer == NULL
      || length == 0) {
    return;
  }
  uint8_t finger = 0;
  uint8_t gpdCommand = gpdCommandBuffer[finger++];
  length--;
  if (sl_zigbee_gpd_af_plugin_incoming_command_cb(gpdCommand,
                                                  length,
                                                  &gpdCommandBuffer[finger])) {
    // Application handled this call hence no need to handle.
    return;
  }
  switch (gpdCommand) {
    case GP_CMD_CHANNEL_CONFIG:
    {
      uint8_t channel = ((gpdCommandBuffer[finger] & 0x0F) + 11);
      sl_zigbee_gpd_af_plugin_commissioning_channel_config_cb(channel);
      break;
    }
    case GP_CMD_COMMISSIONING_REPLY:
    {
      sl_zigbee_gpd_af_plugin_commissioning_reply_cb(length,
                                                     &gpdCommandBuffer[finger]);
      break;
    }
    default:
      break;
  }
  return;
}
#endif
