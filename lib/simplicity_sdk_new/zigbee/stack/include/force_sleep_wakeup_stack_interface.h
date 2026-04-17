/***************************************************************************//**
 * @file force_sleep_wakeup_stack_interface.h
 * @brief Declarations for force sleep wakeup APIs that cross the
 * stack/application boundary.  This header is primarily intended for internal use;
 * see force_sleep_wakeup.h for declarations of the plugin APIs themselves.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef FORCE_SLEEP_WAKEUP_STACK_H
#define FORCE_SLEEP_WAKEUP_STACK_H

#include PLATFORM_HEADER
#include "sl_zigbee_types.h"

void sl_802154_cancel_polls(void);
void sl_802154_purge_transmit_queue(void);
void sl_802154_purge_incoming_queue(void);

void sl_mac_lower_mac_force_sleep(bool sleep);

#endif //FORCE_SLEEP_WAKEUP_STACK_H
