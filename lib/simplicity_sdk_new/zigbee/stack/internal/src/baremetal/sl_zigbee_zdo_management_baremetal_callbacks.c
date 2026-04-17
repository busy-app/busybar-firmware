/***************************************************************************//**
 * @file sl_zigbee_zdo_management_baremetal_callbacks.c
 * @brief internal dispatch for 'sl_zigbee_zdo_management' callbacks as a thin-wrapper
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

void sli_zigbee_stack_beacon_survey_complete_callback(sl_zigbee_zdo_status_t status,
                                                      sl_zigbee_beacon_survey_results_t *survey_results,
                                                      uint8_t potential_parent_count,
                                                      sl_zigbee_potential_parent_t *potential_parents,
                                                      uint16_t pan_id_conflicts)
{
  sl_zigbee_beacon_survey_complete_callback(status,
                                            survey_results,
                                            potential_parent_count,
                                            potential_parents,
                                            pan_id_conflicts);
}
