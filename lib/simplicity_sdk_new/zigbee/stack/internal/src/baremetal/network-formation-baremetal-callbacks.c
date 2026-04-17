/***************************************************************************//**
 * @file network-formation-baremetal-callbacks.c
 * @brief internal dispatch for 'network-formation' callbacks as a thin-wrapper
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
// automatically generated from network-formation.h.  Do not manually edit
#include "stack/include/network-formation.h"
#include "stack/internal/inc/network-formation-internal-def.h"

void sli_zigbee_stack_energy_scan_result_handler(uint8_t channel,
                                                 int8_t maxRssiValue)
{
  sl_zigbee_energy_scan_result_handler(channel,
                                       maxRssiValue);
}

void sli_zigbee_stack_network_found_handler(sl_zigbee_zigbee_network_t *networkFound,
                                            uint8_t lqi,
                                            int8_t rssi)
{
  sl_zigbee_network_found_handler(networkFound,
                                  lqi,
                                  rssi);
}

void sli_zigbee_stack_orphan_notification_handler(sl_802154_long_addr_t longId)
{
  sl_zigbee_orphan_notification_handler(longId);
}

void sli_zigbee_stack_scan_complete_handler(uint8_t channel,
                                            sl_status_t status)
{
  sl_zigbee_scan_complete_handler(channel,
                                  status);
}

void sli_zigbee_stack_unused_pan_id_found_handler(sl_802154_pan_id_t panId,
                                                  uint8_t channel)
{
  sl_zigbee_unused_pan_id_found_handler(panId,
                                        channel);
}
