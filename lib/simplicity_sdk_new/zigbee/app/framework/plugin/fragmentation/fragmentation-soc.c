/***************************************************************************//**
 * @file
 * @brief System on chip specific code for fragmentation.
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
    #include "app/framework/include/af.h"
    #include "app/framework/util/af-main.h"
#else
    #include "sl_zigbee_types.h"
    #include "stack/include/message.h"
#endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "fragmentation.h"
//-----------------------------------------------------------------------------
// Globals

//-----------------------------------------------------------------------------
// Functions

void sli_zigbee_af_fragmentation_platform_init_callback(void)
{
  sl_zigbee_fragment_window_size = SL_ZIGBEE_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE;
}

sl_status_t sli_zigbee_af_fragmentation_send(sli_zigbee_tx_fragmented_packet_t* txPacket,
                                             uint8_t fragmentNumber,
                                             uint8_t fragmentLen,
                                             uint16_t offset)
{
  sl_status_t status;

  status = sl_zigbee_send_unicast(txPacket->messageType,
                                  txPacket->indexOrDestination,
                                  &txPacket->apsFrame,
                                  (uint16_t)fragmentNumber,
                                  fragmentLen,
                                  txPacket->bufferPtr + offset,
                                  &(txPacket->apsFrame.sequence));

  return status;
}

void sli_zigbee_af_fragmentation_handle_source_route(sli_zigbee_tx_fragmented_packet_t* txPacket,
                                                     uint16_t indexOrDestination)
{
  // Nothing to do on SOC.  This will be handled later by the NWK code
  // calling into the application to ask for a source route.
  (void) txPacket;
  (void) indexOrDestination;
}

void sli_zigbee_af_fragmentation_send_reply(sl_802154_short_addr_t sender,
                                            sl_zigbee_aps_frame_t* apsFrame,
                                            sli_zigbee_rx_fragmented_packet_t* rxPacket)
{
  apsFrame->groupId = (HIGH_LOW_TO_INT(rxPacket->fragmentMask,
                                       rxPacket->fragmentBase));
  sl_zigbee_send_reply(sender, apsFrame, 0, NULL);
}
