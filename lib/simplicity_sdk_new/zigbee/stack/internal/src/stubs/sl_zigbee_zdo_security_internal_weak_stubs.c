/***************************************************************************//**
 * @file sl_zigbee_zdo_security_internal_weak_stubs.c
 * @brief stubbed definitions of internal implementations for sl_zigbee_zdo_security
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
// automatically generated from sl_zigbee_zdo_security.h.  Do not manually edit
#include "stack/internal/inc/sl_zigbee_zdo_security_internal_def.h"
#include "sl_common.h"

// Command Indirection

SL_WEAK sl_status_t sli_zigbee_stack_get_authentication_level(sl_802154_short_addr_t dest,
                                                              sl_802154_long_addr_t target)
{
  // NOTE stub definition
  (void) dest;
  (void) target;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_symmetric_passphrase(sl_802154_long_addr_t eui64,
                                                              sl_802154_short_addr_t short_id,
                                                              uint8_t *passphrase)
{
  // NOTE stub definition
  (void) eui64;
  (void) short_id;
  (void) passphrase;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_initiate_security_challenge(sl_802154_short_addr_t partnerNodeId,
                                                                 sl_802154_long_addr_t partnerLong,
                                                                 uint8_t keyIndex)
{
  // NOTE stub definition
  (void) partnerNodeId;
  (void) partnerLong;
  (void) keyIndex;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK void sli_zigbee_stack_retrieve_authentication_token(sl_802154_short_addr_t destination,
                                                            sl_zigbee_aps_option_t options)
{
  // NOTE stub definition
  (void) destination;
  (void) options;
}
