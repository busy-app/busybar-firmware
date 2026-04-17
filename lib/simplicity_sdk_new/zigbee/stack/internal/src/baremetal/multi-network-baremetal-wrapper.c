/***************************************************************************//**
 * @file multi-network-baremetal-wrapper.c
 * @brief internal implementations for 'multi-network' as a thin-wrapper
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
// automatically generated from multi-network.h.  Do not manually edit
#include "stack/include/multi-network.h"
#include "stack/internal/inc/multi-network-internal-def.h"

uint8_t sl_zigbee_get_callback_network(void)
{
  return sli_zigbee_stack_get_callback_network();
}

uint8_t sl_zigbee_get_current_network(void)
{
  return sli_zigbee_stack_get_current_network();
}

sl_status_t sl_zigbee_set_current_network(uint8_t index)
{
  return sli_zigbee_stack_set_current_network(index);
}
