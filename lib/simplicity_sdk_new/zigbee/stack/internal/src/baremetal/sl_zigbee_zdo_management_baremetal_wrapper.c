/***************************************************************************//**
 * @file sl_zigbee_zdo_management_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_zdo_management' as a thin-wrapper
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
// automatically generated from sl_zigbee_zdo_management.h.  Do not manually edit
#include "stack/include/sl_zigbee_zdo_management.h"
#include "stack/internal/inc/sl_zigbee_zdo_management_internal_def.h"

sl_status_t sl_zigbee_request_beacon_survey(sl_802154_short_addr_t dest,
                                            uint8_t channel_page_count,
                                            uint32_t *channel_masks,
                                            uint8_t scan_config_mask)
{
  return sli_zigbee_stack_request_beacon_survey(dest,
                                                channel_page_count,
                                                channel_masks,
                                                scan_config_mask);
}
