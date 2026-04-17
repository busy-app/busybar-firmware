/***************************************************************************//**
 * @file stack-info-baremetal-callbacks.c
 * @brief internal dispatch for 'stack-info' callbacks as a thin-wrapper
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
// automatically generated from stack-info.h.  Do not manually edit
#include "stack/include/stack-info.h"
#include "stack/internal/inc/stack-info-internal-def.h"

void sli_zigbee_stack_radio_needs_calibrating_handler(void)
{
  sl_zigbee_radio_needs_calibrating_handler();
}

void sli_zigbee_stack_stack_status_handler(sl_status_t status)
{
  sl_zigbee_stack_status_handler(status);
}

void sli_zigbee_stack_stack_token_changed_handler(uint16_t tokenAddress)
{
  sl_zigbee_stack_token_changed_handler(tokenAddress);
}
