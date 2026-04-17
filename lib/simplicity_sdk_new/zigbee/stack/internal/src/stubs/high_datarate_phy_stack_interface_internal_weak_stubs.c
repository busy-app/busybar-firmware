/***************************************************************************//**
 * @file high_datarate_phy_stack_interface_internal_weak_stubs.c
 * @brief stubbed definitions of internal implementations for high_datarate_phy_stack_interface
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
// automatically generated from high_datarate_phy_stack_interface.h.  Do not manually edit
#include "stack/internal/inc/high_datarate_phy_stack_interface_internal_def.h"
#include "sl_common.h"

// Command Indirection

SL_WEAK sl_status_t sli_mac_stack_force_tx_after_failed_hdr_phy_cca(uint8_t hdr_csma_attempts,
                                                                    uint32_t min_bo_period_us,
                                                                    uint32_t max_bo_period_us)
{
  // NOTE stub definition
  (void) hdr_csma_attempts;
  (void) min_bo_period_us;
  (void) max_bo_period_us;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK void sli_mac_stack_lower_mac_set_high_datarate_csma_params(sl_rail_csma_config_t *csma_params)
{
  // NOTE stub definition
  (void) csma_params;
}

SL_WEAK void sli_mac_stack_lower_mac_set_high_datarate_phy_radio_priorities(sl_802154_radio_priorities_t *priorities)
{
  // NOTE stub definition
  (void) priorities;
}

SL_WEAK sl_status_t sli_mac_stack_send_raw_high_datarate_phy_message(uint8_t nwk_index,
                                                                     uint8_t *payload)
{
  // NOTE stub definition
  (void) nwk_index;
  (void) payload;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_mac_stack_send_raw_high_datarate_phy_scheduled_message(uint8_t nwk_index,
                                                                               uint8_t *payload,
                                                                               sl_rail_time_t timestamp)
{
  // NOTE stub definition
  (void) nwk_index;
  (void) payload;
  (void) timestamp;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_rail_status_t sli_mac_stack_set_mode_switch_sync_detect(bool enable_f)
{
  // NOTE stub definition
  (void) enable_f;
  return SL_STATUS_NOT_AVAILABLE;
}
