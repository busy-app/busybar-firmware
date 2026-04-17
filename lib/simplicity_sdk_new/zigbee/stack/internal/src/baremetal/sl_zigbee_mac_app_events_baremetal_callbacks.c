/***************************************************************************//**
 * @file sl_zigbee_mac_app_events_baremetal_callbacks.c
 * @brief internal dispatch for 'sl_zigbee_mac_app_events' callbacks as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_mac_app_events.h.  Do not manually edit
#include "stack/mac/sl_zigbee_mac_app_events.h"
#include "stack/internal/inc/sl_zigbee_mac_app_events_internal_def.h"

void sli_802154_stackmac_communication_status_indication_handler(uint8_t mac_index,
                                                                 sl_status_t status,
                                                                 uint8_t packet_length,
                                                                 uint8_t *packet_data)
{
  sl_802154mac_communication_status_indication_handler(mac_index,
                                                       status,
                                                       packet_length,
                                                       packet_data);
}
