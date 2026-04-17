/***************************************************************************//**
 * @file bootload_baremetal_callbacks.c
 * @brief internal dispatch for 'bootload' callbacks as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from bootload.h.  Do not manually edit
#include "stack/include/bootload.h"
#include "stack/internal/inc/bootload_internal_def.h"

void sli_zigbee_stack_bootload_transmit_complete_handler(sl_status_t status,
                                                         uint8_t messageLength,
                                                         uint8_t *messageContents)
{
  sl_zigbee_bootload_transmit_complete_handler(status,
                                               messageLength,
                                               messageContents);
}

void sli_zigbee_stack_incoming_bootload_message_handler(sl_802154_long_addr_t longId,
                                                        sl_zigbee_rx_packet_info_t *packetInfo,
                                                        uint8_t messageLength,
                                                        uint8_t *messageContents)
{
  sl_zigbee_incoming_bootload_message_handler(longId,
                                              packetInfo,
                                              messageLength,
                                              messageContents);
}
