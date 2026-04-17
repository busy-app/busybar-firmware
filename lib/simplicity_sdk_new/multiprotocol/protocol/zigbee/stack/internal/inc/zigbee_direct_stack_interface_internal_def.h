/***************************************************************************//**
 * @file zigbee_direct_stack_interface_internal_def.h
 * @brief internal names for 'zigbee_direct_stack_interface' declarations
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
// automatically generated from zigbee_direct_stack_interface.h.  Do not manually edit
#ifndef ZIGBEE_DIRECT_STACK_INTERFACE_INTERNAL_DEF_H
#define ZIGBEE_DIRECT_STACK_INTERFACE_INTERNAL_DEF_H

#include "stack/include/zigbee_direct_stack_interface.h"

// Command Indirection

void sli_zigbee_stack_direct_send_commissioning_response(uint8_t status,
                                                         sl_802154_short_addr_t dst,
                                                         sl_802154_long_addr_t longDest);

bool sli_zigbee_stack_direct_send_ephemeral_key(sl_802154_short_addr_t destinationShortId,
                                                sl_802154_long_addr_t destinationLongId,
                                                sl_802154_long_addr_t sourceOrPartnerLongId,
                                                uint8_t keyType,
                                                const sl_zigbee_key_data_t *keyData,
                                                uint8_t options);

uint8_t sli_zigbee_stack_get_nwk_update_id(void);

sl_status_t sli_zigbee_stack_transient_device_mgmt_finish(const sl_zigbee_address_info *device_ids);

#endif // ZIGBEE_DIRECT_STACK_INTERFACE_INTERNAL_DEF_H
