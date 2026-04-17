/***************************************************************************//**
 * @file child_baremetal_callbacks.c
 * @brief internal dispatch for 'child' callbacks as a thin-wrapper
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
// automatically generated from child.h.  Do not manually edit
#include "stack/include/child.h"
#include "stack/internal/inc/child_internal_def.h"

void sli_zigbee_stack_child_join_handler(uint8_t index,
                                         bool joining,
                                         sl_802154_short_addr_t childId,
                                         sl_802154_long_addr_t childEui64,
                                         sl_zigbee_node_type_t childType)
{
  sl_zigbee_child_join_handler(index,
                               joining,
                               childId,
                               childEui64,
                               childType);
}

void sli_zigbee_stack_poll_complete_handler(sl_status_t status)
{
  sl_zigbee_poll_complete_handler(status);
}

void sli_zigbee_stack_poll_handler(sl_802154_short_addr_t childId,
                                   bool transmitExpected)
{
  sl_zigbee_poll_handler(childId,
                         transmitExpected);
}
