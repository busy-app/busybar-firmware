/***************************************************************************//**
 * @file sl_zigbee_token_internal_weak_stubs.c
 * @brief stubbed definitions of internal implementations for sl_zigbee_token
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
// automatically generated from sl_zigbee_token.h.  Do not manually edit
#include "stack/internal/inc/sl_zigbee_token_internal_def.h"
#include "sl_common.h"

// Command Indirection

SL_WEAK uint32_t sli_zigbee_stack_get_token_count(void)
{
  // NOTE stub definition

  return 0;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_token_data(uint32_t token,
                                                    uint32_t index,
                                                    sl_zigbee_token_data_t *tokenData)
{
  // NOTE stub definition
  (void) token;
  (void) index;
  (void) tokenData;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_get_token_info(uint8_t index,
                                                    sl_zigbee_token_info_t *tokenInfo)
{
  // NOTE stub definition
  (void) index;
  (void) tokenInfo;
  return SL_STATUS_NOT_AVAILABLE;
}

SL_WEAK sl_status_t sli_zigbee_stack_set_token_data(uint32_t token,
                                                    uint32_t index,
                                                    sl_zigbee_token_data_t *tokenData)
{
  // NOTE stub definition
  (void) token;
  (void) index;
  (void) tokenData;
  return SL_STATUS_NOT_AVAILABLE;
}
