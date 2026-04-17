/***************************************************************************//**
 * @brief ZigBee token legacy code.
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

#if !defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST) && !defined(ZIGBEE_STACK_ON_HOST)
#include "sl_token_api.h"
#else
#include "token.h"
#endif
#include "sl_token_manager.h"
#include "sl_zigbee_token.h"
#include "sl_zigbee_types.h"

extern const uint32_t tokenNvm3Keys[];
extern const bool tokenIsIdx[];
extern const bool tokenIsCnt[];
extern const uint8_t tokenSize[];
extern const uint8_t tokenArraySize[];
extern const void * const tokenDefaults[];

// From the token interface
uint32_t sli_zigbee_stack_get_token_count(void)
{
  return TOKEN_COUNT;
}

sl_status_t sl_zigbee_get_token_default(uint32_t token,
                                        uint8_t *default_token_value)
{
  for (uint32_t token_idx = 0; token_idx < TOKEN_COUNT; token_idx++) {
    if (token == tokenNvm3Keys[token_idx]) {
      memcpy(default_token_value, tokenDefaults[token_idx], tokenSize[token_idx]);
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t sli_zigbee_stack_get_token_info(uint8_t index,
                                            sl_zigbee_token_info_t *tokenInfo)
{
  if (index >= sli_zigbee_stack_get_token_count()) {
    return SL_STATUS_INVALID_INDEX;
  }
  tokenInfo->nvm3Key = tokenNvm3Keys[index];
  tokenInfo->isIdx = 1;
  tokenInfo->isCnt = tokenIsCnt[index];
  tokenInfo->size = (uint32_t) tokenSize[index];
  tokenInfo->arraySize = tokenArraySize[index];
  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_stack_get_token_data(uint32_t token,
                                            uint32_t index,
                                            sl_zigbee_token_data_t *tokenData)
{
  // Look up the token size from the token key because
  // sl_token_get_data needs the size to be passed.
  for (uint8_t i = 0; i < sli_zigbee_stack_get_token_count(); i++) {
    if (token == tokenNvm3Keys[i]) {
      tokenData->size = tokenSize[i];
      return sl_token_get_data(token,
                               index,
                               tokenData->data,
                               tokenData->size);
    }
  }
  tokenData->size = 0;
  return SL_STATUS_FAIL;
}

sl_status_t sli_zigbee_stack_set_token_data(uint32_t token,
                                            uint32_t index,
                                            sl_zigbee_token_data_t *tokenData)
{
  sl_status_t status = sl_token_set_data(token,
                                         index,
                                         tokenData->data,
                                         tokenData->size);
  if (status != SL_STATUS_OK) {
    tokenData->size = 0;
  }
  return status;
}

/* When using legacy token manager (token_manager.slcc), we don't want these
 * initializing functions to do anything, since token initialization is handled
 * by sl_token_def.c
 */
sl_status_t sl_zigbee_initialize_app_tokens(void)
{
  return SL_STATUS_OK;
}
sl_status_t sl_zigbee_initialize_basic_token(uint32_t token,
                                             void *default_token_value,
                                             uint32_t token_size)
{
  (void)token;
  (void)default_token_value;
  (void)token_size;
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_initialize_counter_token(uint32_t token,
                                               void *default_token_value,
                                               uint32_t token_size)
{
  (void)token;
  (void)default_token_value;
  (void)token_size;
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_initialize_index_token(uint32_t token_base,
                                             void *default_token_value,
                                             uint32_t token_size,
                                             uint8_t token_index_size)
{
  (void)token_base;
  (void)default_token_value;
  (void)token_size;
  (void)token_index_size;
  return SL_STATUS_OK;
}
