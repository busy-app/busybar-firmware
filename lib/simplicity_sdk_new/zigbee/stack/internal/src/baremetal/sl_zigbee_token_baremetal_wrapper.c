/***************************************************************************//**
 * @file sl_zigbee_token_baremetal_wrapper.c
 * @brief internal implementations for 'sl_zigbee_token' as a thin-wrapper
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
#include "stack/include/sl_zigbee_token.h"
#include "stack/internal/inc/sl_zigbee_token_internal_def.h"

uint32_t sl_zigbee_get_token_count(void)
{
  return sli_zigbee_stack_get_token_count();
}

sl_status_t sl_zigbee_get_token_data(uint32_t token,
                                     uint32_t index,
                                     sl_zigbee_token_data_t *tokenData)
{
  return sli_zigbee_stack_get_token_data(token,
                                         index,
                                         tokenData);
}

sl_status_t sl_zigbee_get_token_info(uint8_t index,
                                     sl_zigbee_token_info_t *tokenInfo)
{
  return sli_zigbee_stack_get_token_info(index,
                                         tokenInfo);
}

sl_status_t sl_zigbee_set_token_data(uint32_t token,
                                     uint32_t index,
                                     sl_zigbee_token_data_t *tokenData)
{
  return sli_zigbee_stack_set_token_data(token,
                                         index,
                                         tokenData);
}
