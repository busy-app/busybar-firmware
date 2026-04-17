/***************************************************************************//**
 * @file mac-layer-baremetal-wrapper.c
 * @brief internal implementations for 'mac-layer' as a thin-wrapper
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
// automatically generated from mac-layer.h.  Do not manually edit
#include "stack/include/mac-layer.h"
#include "stack/internal/inc/mac-layer-internal-def.h"

sl_status_t sl_zigbee_force_tx_after_failed_cca(uint8_t csmaAttempts)
{
  return sli_zigbee_stack_force_tx_after_failed_cca(csmaAttempts);
}

uint8_t sl_zigbee_get_max_mac_retries(void)
{
  return sli_zigbee_stack_get_max_mac_retries();
}
