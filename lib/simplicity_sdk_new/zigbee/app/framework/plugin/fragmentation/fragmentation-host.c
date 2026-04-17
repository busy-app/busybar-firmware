/***************************************************************************//**
 * @file
 * @brief Host specific code for fragmentation.
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

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "fragmentation.h"

//-----------------------------------------------------------------------------
// Globals

uint16_t sl_zigbee_mac_indirect_timeout = 0;
uint16_t sl_zigbee_aps_ack_timeout_ms    = 0;
uint8_t  sl_zigbee_fragment_window_size = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE;

//-----------------------------------------------------------------------------
// Functions

void sl_zigbee_af_fragmentation_ncp_init_cb(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT,
                                         &sl_zigbee_mac_indirect_timeout);
  sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_APS_ACK_TIMEOUT, &sl_zigbee_aps_ack_timeout_ms);
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_FRAGMENT_WINDOW_SIZE,
                                     sl_zigbee_fragment_window_size,
                                     "Fragmentation RX window size");
}

void sli_zigbee_af_fragmentation_platform_init_callback(void)
{
}

sl_status_t sli_zigbee_af_fragmentation_send(sli_zigbee_tx_fragmented_packet_t* txPacket,
                                             uint8_t fragmentNumber,
                                             uint8_t fragmentLen,
                                             uint16_t offset)
{
  sl_status_t status;

  status = sl_zigbee_send_unicast(txPacket->messageType,
                                  txPacket->indexOrDestination,
                                  &(txPacket->apsFrame),
                                  (uint16_t)fragmentNumber,
                                  fragmentLen,
                                  txPacket->bufferPtr + offset,
                                  &(txPacket->apsFrame.sequence));
  return status;
}

void sli_zigbee_af_fragmentation_handle_source_route(sli_zigbee_tx_fragmented_packet_t* txPacket,
                                                     uint16_t indexOrDestination)
{
  (void)txPacket;
  (void)indexOrDestination;
  // Host: no source-route handling needed here; same as SoC.
}

void sli_zigbee_af_fragmentation_send_reply(sl_802154_short_addr_t sender,
                                            sl_zigbee_aps_frame_t* apsFrame,
                                            sli_zigbee_rx_fragmented_packet_t* rxPacket)
{
  apsFrame->groupId = HIGH_LOW_TO_INT(rxPacket->fragmentMask, rxPacket->fragmentBase);
  sl_zigbee_ezsp_send_reply(sender, apsFrame, 0, NULL);
}
