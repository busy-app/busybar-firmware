/***************************************************************************//**
 * @file security_baremetal_callbacks.c
 * @brief internal dispatch for 'security' callbacks as a thin-wrapper
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
// automatically generated from security.h.  Do not manually edit
#include "stack/include/security.h"
#include "stack/internal/inc/security_internal_def.h"

void sli_zigbee_stack_switch_network_key_handler(uint8_t sequenceNumber)
{
  sl_zigbee_switch_network_key_handler(sequenceNumber);
}

void sli_zigbee_stack_zigbee_key_establishment_handler(sl_802154_long_addr_t partner,
                                                       sl_zigbee_key_status_t status)
{
  sl_zigbee_zigbee_key_establishment_handler(partner,
                                             status);
}
