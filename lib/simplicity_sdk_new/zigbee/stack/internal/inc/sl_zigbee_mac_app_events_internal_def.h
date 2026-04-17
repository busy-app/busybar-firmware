/***************************************************************************//**
 * @file sl_zigbee_mac_app_events_internal_def.h
 * @brief internal names for 'sl_zigbee_mac_app_events' declarations
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
#ifndef SL_ZIGBEE_MAC_APP_EVENTS_INTERNAL_DEF_H
#define SL_ZIGBEE_MAC_APP_EVENTS_INTERNAL_DEF_H

#include "stack/mac/sl_zigbee_mac_app_events.h"

// Callback Indirection

void sli_802154_stackmac_communication_status_indication_handler(uint8_t mac_index,
                                                                 sl_status_t status,
                                                                 uint8_t packet_length,
                                                                 uint8_t *packet_data);

#endif // SL_ZIGBEE_MAC_APP_EVENTS_INTERNAL_DEF_H
