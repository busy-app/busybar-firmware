/***************************************************************************//**
 * @file zigbee_direct_stack_interface_internal_weak_stubs.c
 * @brief stubbed definitions of internal implementations for zigbee_direct_stack_interface
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
// automatically generated from zigbee_direct_stack_interface.h.  Do not manually edit
#include "stack/internal/inc/zigbee_direct_stack_interface_internal_def.h"
#include "sl_common.h"

// Command Indirection

SL_WEAK void sli_zigbee_stack_direct_send_commissioning_response(uint8_t status,
                                                                 sl_802154_short_addr_t dst,
                                                                 sl_802154_long_addr_t longDest)
{
  // NOTE stub definition
  (void) status;
  (void) dst;
  (void) longDest;
}

SL_WEAK bool sli_zigbee_stack_direct_send_ephemeral_key(sl_802154_short_addr_t destinationShortId,
                                                        sl_802154_long_addr_t destinationLongId,
                                                        sl_802154_long_addr_t sourceOrPartnerLongId,
                                                        uint8_t keyType,
                                                        const sl_zigbee_key_data_t *keyData,
                                                        uint8_t options)
{
  // NOTE stub definition
  (void) destinationShortId;
  (void) destinationLongId;
  (void) sourceOrPartnerLongId;
  (void) keyType;
  (void) keyData;
  (void) options;
  return false;
}

SL_WEAK uint8_t sli_zigbee_stack_get_nwk_update_id(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t sli_zigbee_stack_transient_device_mgmt_finish(const sl_zigbee_address_info *device_ids)
{
  // NOTE stub definition
  (void) device_ids;
  return SL_STATUS_NOT_AVAILABLE;
}
