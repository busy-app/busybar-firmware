/***************************************************************************//**
 * @file force_sleep_wakeup_stack_interface_internal_weak_stubs.c
 * @brief stubbed definitions of internal implementations for force_sleep_wakeup_stack_interface
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
// automatically generated from force_sleep_wakeup_stack_interface.h.  Do not manually edit
#include "stack/internal/inc/force_sleep_wakeup_stack_interface_internal_def.h"
#include "sl_common.h"

// Command Indirection

SL_WEAK void sli_802154_stack_cancel_polls(void)
{
  // NOTE stub definition
}

SL_WEAK void sli_802154_stack_purge_incoming_queue(void)
{
  // NOTE stub definition
}

SL_WEAK void sli_802154_stack_purge_transmit_queue(void)
{
  // NOTE stub definition
}

SL_WEAK void sli_mac_stack_lower_mac_force_sleep(bool sleep)
{
  // NOTE stub definition
  (void) sleep;
}
