/***************************************************************************//**
 * @file zigbee_direct_stack_interface.h
 * @brief Declarations for Zigbee Direct APIs that cross the
 * stack/application boundary.  This header is primarily intended for internal use
 * by the Zigbee Direct plugin code.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef ZIGBEE_DIRECT_STACK_H
#define ZIGBEE_DIRECT_STACK_H

#include PLATFORM_HEADER
#include "sl_zigbee_types.h"
#include "sl_zigbee_address_info.h"
#include "zigbee_packet_types.h"

uint8_t sl_zigbee_get_nwk_update_id(void);

sl_status_t sl_zigbee_transient_device_mgmt_finish(const sl_zigbee_address_info *device_ids);

bool sl_zigbee_direct_send_ephemeral_key(sl_802154_short_addr_t destinationShortId,
                                         sl_802154_long_addr_t destinationLongId,
                                         sl_802154_long_addr_t sourceOrPartnerLongId,
                                         uint8_t keyType,
                                         const sl_zigbee_key_data_t* keyData,
                                         uint8_t options);

void sl_zigbee_direct_send_commissioning_response(uint8_t status,
                                                  sl_802154_short_addr_t dst,
                                                  sl_802154_long_addr_t longDest);

#endif //ZIGBEE_DIRECT_STACK_H
