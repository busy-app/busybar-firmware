/***************************************************************************//**
 * @file high_datarate_phy_stack_interface_baremetal_callbacks.c
 * @brief internal dispatch for 'high_datarate_phy_stack_interface' callbacks as a thin-wrapper
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
// automatically generated from high_datarate_phy_stack_interface.h.  Do not manually edit
#include "stack/include/high_datarate_phy_stack_interface.h"
#include "stack/internal/inc/high_datarate_phy_stack_interface_internal_def.h"

void sli_mac_stack_high_datarate_phy_rx_callback(uint8_t *packet,
                                                 uint8_t linkQuality,
                                                 int8_t rssi,
                                                 uint32_t pkt_rx_timestamp)
{
  sl_mac_high_datarate_phy_rx_callback(packet,
                                       linkQuality,
                                       rssi,
                                       pkt_rx_timestamp);
}

void sli_mac_stack_high_datarate_phy_tx_callback(uint8_t mac_index,
                                                 sl_status_t status,
                                                 uint16_t packet_length,
                                                 uint8_t *packet_contents,
                                                 uint8_t tag)
{
  sl_mac_high_datarate_phy_tx_callback(mac_index,
                                       status,
                                       packet_length,
                                       packet_contents,
                                       tag);
}
