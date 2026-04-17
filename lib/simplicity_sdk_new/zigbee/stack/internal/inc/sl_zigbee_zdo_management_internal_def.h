/***************************************************************************//**
 * @file sl_zigbee_zdo_management_internal_def.h
 * @brief internal names for 'sl_zigbee_zdo_management' declarations
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
#ifndef SL_ZIGBEE_ZDO_MANAGEMENT_INTERNAL_DEF_H
#define SL_ZIGBEE_ZDO_MANAGEMENT_INTERNAL_DEF_H

#include "stack/include/sl_zigbee_zdo_management.h"

// Command Indirection

sl_status_t sli_zigbee_stack_request_beacon_survey(sl_802154_short_addr_t dest,
                                                   uint8_t channel_page_count,
                                                   uint32_t *channel_masks,
                                                   uint8_t scan_config_mask);

// Callback Indirection

void sli_zigbee_stack_beacon_survey_complete_callback(sl_zigbee_zdo_status_t status,
                                                      sl_zigbee_beacon_survey_results_t *survey_results,
                                                      uint8_t potential_parent_count,
                                                      sl_zigbee_potential_parent_t *potential_parents,
                                                      uint16_t pan_id_conflicts);

#endif // SL_ZIGBEE_ZDO_MANAGEMENT_INTERNAL_DEF_H
