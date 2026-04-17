/***************************************************************************//**
 * @file force_sleep_wakeup_stack_interface_internal_def.h
 * @brief internal names for 'force_sleep_wakeup_stack_interface' declarations
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
#ifndef FORCE_SLEEP_WAKEUP_STACK_INTERFACE_INTERNAL_DEF_H
#define FORCE_SLEEP_WAKEUP_STACK_INTERFACE_INTERNAL_DEF_H

#include "stack/include/force_sleep_wakeup_stack_interface.h"

// Command Indirection

void sli_802154_stack_cancel_polls(void);

void sli_802154_stack_purge_incoming_queue(void);

void sli_802154_stack_purge_transmit_queue(void);

void sli_mac_stack_lower_mac_force_sleep(bool sleep);

#endif // FORCE_SLEEP_WAKEUP_STACK_INTERFACE_INTERNAL_DEF_H
