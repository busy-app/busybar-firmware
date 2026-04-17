/***************************************************************************//**
 * @file bootload_baremetal_wrapper.c
 * @brief internal implementations for 'bootload' as a thin-wrapper
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
// automatically generated from bootload.h.  Do not manually edit
#include "stack/include/bootload.h"
#include "stack/internal/inc/bootload_internal_def.h"

sl_status_t sl_zigbee_send_bootload_message(bool broadcast,
                                            sl_802154_long_addr_t destEui64,
                                            uint8_t messageLength,
                                            uint8_t *messageContents)
{
  return sli_zigbee_stack_send_bootload_message(broadcast,
                                                destEui64,
                                                messageLength,
                                                messageContents);
}
