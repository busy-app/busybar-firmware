/***************************************************************************//**
 * @file sl_zigbee_address_info_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_address_info' as a thin-wrapper
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
// automatically generated from sl_zigbee_address_info.h.  Do not manually edit
#include "stack/include/sl_zigbee_address_info.h"
#include "stack/internal/inc/sl_zigbee_address_info_internal_def.h"

bool sl_zigbee_device_id_matches(const sl_zigbee_address_info *set_a,
                                 const sl_zigbee_address_info *set_b)
{
  return sli_zigbee_stack_device_id_matches(set_a,
                                            set_b);
}

sl_zigbee_address_info sl_zigbee_make_device_id_pair(const sl_802154_short_addr_t short_id,
                                                     const sl_802154_long_addr_t long_id)
{
  return sli_zigbee_stack_make_device_id_pair(short_id,
                                              long_id);
}
