/***************************************************************************//**
 * @file force_sleep_wakeup_stack_interface_baremetal_wrapper.c
 * @brief internal implementations for 'force_sleep_wakeup_stack_interface' as a thin-wrapper
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
// automatically generated from force_sleep_wakeup_stack_interface.h.  Do not manually edit
#include "stack/include/force_sleep_wakeup_stack_interface.h"
#include "stack/internal/inc/force_sleep_wakeup_stack_interface_internal_def.h"

void sl_802154_cancel_polls(void)
{
  sli_802154_stack_cancel_polls();
}

void sl_802154_purge_incoming_queue(void)
{
  sli_802154_stack_purge_incoming_queue();
}

void sl_802154_purge_transmit_queue(void)
{
  sli_802154_stack_purge_transmit_queue();
}

void sl_mac_lower_mac_force_sleep(bool sleep)
{
  sli_mac_stack_lower_mac_force_sleep(sleep);
}
