/***************************************************************************//**
 * @file sl_zigbee_zdo_security_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_zdo_security' as a thin-wrapper
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
// automatically generated from sl_zigbee_zdo_security.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_security.h"
#include "stack/internal/inc/sl_zigbee_zdo_security_internal_def.h"

sl_status_t sl_zigbee_get_authentication_level(sl_802154_short_addr_t dest,
                                               sl_802154_long_addr_t target)
{
  return sli_zigbee_stack_get_authentication_level(dest,
                                                   target);
}

sl_status_t sl_zigbee_get_symmetric_passphrase(sl_802154_long_addr_t eui64,
                                               sl_802154_short_addr_t short_id,
                                               uint8_t *passphrase)
{
  return sli_zigbee_stack_get_symmetric_passphrase(eui64,
                                                   short_id,
                                                   passphrase);
}

sl_status_t sl_zigbee_initiate_security_challenge(sl_802154_short_addr_t partnerNodeId,
                                                  sl_802154_long_addr_t partnerLong,
                                                  uint8_t keyIndex)
{
  return sli_zigbee_stack_initiate_security_challenge(partnerNodeId,
                                                      partnerLong,
                                                      keyIndex);
}

void sl_zigbee_retrieve_authentication_token(sl_802154_short_addr_t destination,
                                             sl_zigbee_aps_option_t options)
{
  sli_zigbee_stack_retrieve_authentication_token(destination,
                                                 options);
}
