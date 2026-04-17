/***************************************************************************//**
 * @brief ZigBee token management code.
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

#include "sl_zigbee_token.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_zigbee_types.h"
#include "sl_token_manager_defines.h"
#include "sl_token_manager_api.h"
#if !defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST) && !defined(ZIGBEE_STACK_ON_HOST)
#include "nvm3.h"
#else // !EZSP_HOST && !SL_ZIGBEE_TEST && !ZIGBEE_STACK_ON_HOST
// TODO how do we get this define on the host?
#define NVM3_KEY_SIZE               20U                           ///< Unique object key identifier size in number of bits
#define NVM3_KEY_MASK               ((1U << NVM3_KEY_SIZE) - 1U)  ///< Unique object key identifier mask

extern sl_status_t sli_zigbee_host_token_init(uint32_t token_base,
                                              void *default_token_value,
                                              uint32_t token_size,
                                              uint8_t token_index_size,
                                              bool token_is_counter);
#endif // EZSP_HOST

#include <stdlib.h>
#include <stdio.h>
// -------------------------------------------------------------------------
// ------------------------------OLD APIS-----------------------------------
// -------------------------------------------------------------------------

//-- Build parameter links
#define DEFINETOKENS
#define TOKEN_MFG(name, creator, iscnt, isidx, type, arraysize, ...)

#if defined(APP_FRAMEWORK_TOKEN_HEADER)

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  NVM3KEY_##name,
const uint32_t tokenAfNvm3Keys[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
  #include APP_FRAMEWORK_TOKEN_HEADER
};

#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  creator,
const uint16_t tokenAfCreators[] = {
  0xFFFFUL   // Dummy
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  iscnt,
const bool tokenAfIsCnt[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APP_FRAMEWORK_TOKEN_HEADER
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  isidx,
const bool tokenAfIsIdx[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APP_FRAMEWORK_TOKEN_HEADER
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  sizeof(type),
const uint8_t tokenAfSize[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APP_FRAMEWORK_TOKEN_HEADER
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  arraysize,
const uint8_t tokenAfArraySize[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APP_FRAMEWORK_TOKEN_HEADER
};
#undef TOKEN_DEF

//These set of DEFAULT token values are only used in the tokenDefaults array
//below.

// -------------------------------------------------------------------------
// MISRAC2012-Rule-20.7 doesn't allow for this use of concatenation within
// a macro expansion. Moreover, since TOKEN_DEF is only being used here as a
// static declaration of token default values, there's no risk that it will
// interfere with any other logic.
// -------------------------------------------------------------------------
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  const type TOKEN_##name##_DEFAULTS = __VA_ARGS__;
// Multiple inclusion of unguarded token-related header files is by design; suppress violation.
//cstat !MISRAC2012-Dir-4.10
  #include APP_FRAMEWORK_TOKEN_HEADER
#undef TOKEN_DEF
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  ((void *)&TOKEN_##name##_DEFAULTS),
void * tokenAfDefaults[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APP_FRAMEWORK_TOKEN_HEADER
};
#undef TOKEN_DEF

#endif // APP_FRAMEWORK_TOKEN_HEADER

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
#if defined(APPLICATION_TOKEN_HEADER)

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  NVM3KEY_##name,
const uint32_t tokenAppNvm3Keys[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
  #include APPLICATION_TOKEN_HEADER
};

#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  creator,
const uint16_t tokenAppCreators[] = {
  0xFFFFUL   // Dummy
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  iscnt,
const bool tokenAppIsCnt[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APPLICATION_TOKEN_HEADER
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  isidx,
const bool tokenAppIsIdx[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APPLICATION_TOKEN_HEADER
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  sizeof(type),
const uint8_t tokenAppSize[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APPLICATION_TOKEN_HEADER
};
#undef TOKEN_DEF

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  arraysize,
const uint8_t tokenAppArraySize[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APPLICATION_TOKEN_HEADER
};
#undef TOKEN_DEF

//These set of DEFAULT token values are only used in the tokenDefaults array
//below.

// -------------------------------------------------------------------------
// MISRAC2012-Rule-20.7 doesn't allow for this use of concatenation within
// a macro expansion. Moreover, since TOKEN_DEF is only being used here as a
// static declaration of token default values, there's no risk that it will
// interfere with any other logic.
// -------------------------------------------------------------------------
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  const type TOKEN_##name##_DEFAULTS = __VA_ARGS__;
// Multiple inclusion of unguarded token-related header files is by design; suppress violation.
//cstat !MISRAC2012-Dir-4.10
  #include APPLICATION_TOKEN_HEADER
#undef TOKEN_DEF
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  ((void *)&TOKEN_##name##_DEFAULTS),
const void * const tokenAppDefaults[] = {
  // Multiple inclusion of unguarded token-related header files is by design; suppress violation.
  //cstat !MISRAC2012-Dir-4.10
    #include APPLICATION_TOKEN_HEADER
};
#undef TOKEN_DEF

#endif // APPLICATION_TOKEN_HEADER

#endif // SL_ZIGBEE_SCRIPTED_TEST

#undef DEFINETOKENS

#undef TOKEN_MFG

// -------------------------------------------------------------------------
// ------------------------------NEW APIS-----------------------------------
// -------------------------------------------------------------------------

uint32_t *tokenNvm3Keys = NULL;
bool *tokenIsCnt = NULL;
bool *tokenIsIdx = NULL;
uint32_t *tokenSize = NULL;
uint8_t *tokenArraySize = NULL;
uint8_t *tokenDefaults = NULL;

uint32_t sli_zigbee_token_count = 0;
// total size of all tokens, including each index's size
uint32_t sli_zigbee_token_total_size = 0;
// total size of all tokens, but ignores nonzero index sizes (e.g. it only considers the base token's size)
uint32_t sli_zigbee_base_token_total_size = 0;

// ------------------------Forward Declarations-----------------------------
sl_status_t sli_zigbee_stack_initialize_token(uint32_t token_base,
                                              void *default_token_value,
                                              uint32_t token_size,
                                              uint8_t token_index_size,
                                              bool token_is_counter);

uint32_t sli_zigbee_stack_get_token_count(void)
{
  return sli_zigbee_token_count;
}

// Note: this only works where token is a base indexed token ID, like COMMON_TOKEN_STACK_BINDING_TABLE
// It does not work if you pass in one of the indices, like COMMON_TOKEN_STACK_BINDING_TABLE+1
sl_status_t sl_zigbee_get_token_default(uint32_t base_token,
                                        uint8_t *default_token_value)
{
  if (tokenNvm3Keys == NULL) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  uint32_t running_token_size = 0;
  for (uint32_t i = 0; i < sli_zigbee_token_count; i++) {
    uint32_t token_size = tokenSize[i];
    if (tokenNvm3Keys[i] == base_token) {
      memcpy(default_token_value, tokenDefaults + running_token_size, token_size);
      return SL_STATUS_OK;
    }
    running_token_size += token_size;
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
  tokenInfo->isIdx = tokenIsIdx[index];
  tokenInfo->isCnt = tokenIsCnt[index];
  tokenInfo->size = tokenSize[index];
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
      if (tokenIsCnt[i]) {
        token |= SL_TOKEN_NVM3_OBJ_TYPE_BIT_MASK;
      }
      tokenData->size = tokenSize[i];
      return sl_token_manager_get_data(token + index,
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
  for (uint8_t i = 0; i < sli_zigbee_stack_get_token_count(); i++) {
    if ((token == tokenNvm3Keys[i]) && (tokenIsCnt[i])) {
      token |= SL_TOKEN_NVM3_OBJ_TYPE_BIT_MASK;
      break;
    }
  }
  sl_status_t status = sl_token_manager_set_data(token + index,
                                                 tokenData->data,
                                                 tokenData->size);
  if (status != SL_STATUS_OK) {
    tokenData->size = 0;
  }
  return status;
}

sl_status_t sl_zigbee_initialize_app_tokens(void)
{
  sl_status_t status = SL_STATUS_OK;

#if defined(APP_FRAMEWORK_TOKEN_HEADER)
  if (APP_FRAMEWORK_TOKEN_COUNT) {
    for (int i = 0; i < APP_FRAMEWORK_TOKEN_COUNT; i++) {
      uint32_t token_manager_klv_key = SL_TOKEN_GET_DYNAMIC_TOKEN(tokenAfNvm3Keys[i], tokenAfIsCnt[i]);
      status = sli_zigbee_stack_initialize_token(token_manager_klv_key,
                                                 tokenAfDefaults[i],
                                                 (uint32_t)tokenAfSize[i],
                                                 tokenAfArraySize[i],
                                                 tokenAfIsCnt[i]);
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
#endif // defined(APP_FRAMEWORK_TOKEN_HEADER)
#if defined(SL_ZIGBEE_SCRIPTED_TEST)
#if defined(APPLICATION_TOKEN_HEADER)
  if (APPLICATION_TOKEN_COUNT) {
    for (int i = 0; i < APPLICATION_TOKEN_COUNT; i++) {
      uint32_t token_manager_klv_key = SL_TOKEN_GET_DYNAMIC_TOKEN(tokenAppNvm3Keys[i], tokenAppIsCnt[i]);
      status = sli_zigbee_stack_initialize_token(token_manager_klv_key,
                                                 (void *)tokenAppDefaults[i],
                                                 (uint32_t)tokenAppSize[i],
                                                 tokenAppArraySize[i],
                                                 tokenAppIsCnt[i]);
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
#endif // defined(APPLICATION_TOKEN_HEADER)
#endif // SL_ZIGBEE_SCRIPTED_TEST
  return status;
}

sl_status_t sli_zigbee_stack_initialize_token(uint32_t token_base,
                                              void *default_token_value,
                                              uint32_t token_size,
                                              uint8_t token_index_size,
                                              bool token_is_counter)
{
  sl_status_t status = SL_STATUS_OK;

  uint32_t nvm3_key = token_base & NVM3_KEY_MASK;

  // If 0 tokens are being made, such as with the case when a configuration item is set
  // to 0 (like SL_ZIGBEE_CHILD_TABLE_SIZE being 0 on a host app), then we do not need
  // to initialize anything
  if (token_index_size == 0) {
    return SL_STATUS_OK;
  }

  // First check if token already exists
  if (tokenNvm3Keys != NULL) {
    for (uint8_t i = 0; i < sli_zigbee_token_count; i++) {
      if (tokenNvm3Keys[i] == nvm3_key) {
        return SL_STATUS_OK;
      }
    }
  }

  // It does not exist, so register it with our arrays. Later we check flash contents
  sli_zigbee_token_count++;

  tokenNvm3Keys = (uint32_t *)realloc(tokenNvm3Keys, (sizeof(uint32_t) * sli_zigbee_token_count));
  assert(tokenNvm3Keys != NULL);

  tokenIsCnt = (bool *)realloc(tokenIsCnt, (sizeof(bool) * sli_zigbee_token_count));
  assert(tokenIsCnt != NULL);

  tokenIsIdx = (bool *)realloc(tokenIsIdx, (sizeof(bool) * sli_zigbee_token_count));
  assert(tokenIsIdx != NULL);

  tokenSize = (uint32_t *)realloc(tokenSize, (sizeof(uint32_t) * sli_zigbee_token_count));
  assert(tokenSize != NULL);

  tokenArraySize = (uint8_t *)realloc(tokenArraySize, (sizeof(uint8_t) * sli_zigbee_token_count));
  assert(tokenArraySize != NULL);

  tokenDefaults = (uint8_t *)realloc(tokenDefaults, (sizeof(uint8_t) * (sli_zigbee_base_token_total_size + token_size)));
  assert(tokenDefaults != NULL);

  tokenNvm3Keys[sli_zigbee_token_count - 1] = nvm3_key;
  tokenIsCnt[sli_zigbee_token_count - 1] = token_is_counter;
  tokenIsIdx[sli_zigbee_token_count - 1] = (bool) (token_index_size != 1);
  tokenSize[sli_zigbee_token_count - 1] = token_size;
  tokenArraySize[sli_zigbee_token_count - 1] = token_index_size;

  memcpy(tokenDefaults + sli_zigbee_base_token_total_size, default_token_value, token_size);
  sli_zigbee_token_total_size += (token_size * token_index_size);
  sli_zigbee_base_token_total_size += token_size;
#if defined(SL_ZIGBEE_TEST)
  // for unit tests and simulation, we don't want to call sl_token_manager_set_data until the simulated EEPRON
  // (tokens in ram) exists, and that cannot be malloc-ed until the total number of tokens is known.
  status = SL_STATUS_OK;
#elif !defined(EZSP_HOST) && !defined(ZIGBEE_STACK_ON_HOST)
  void * current_token_value = malloc(token_size);
  // If it doesn't exist in flash, push it
  status = sl_token_manager_get_data(token_base, current_token_value, token_size);
  free(current_token_value);
  if (status != SL_STATUS_NOT_FOUND) {
    return status;
  }

  // reset the status to OK
  status = SL_STATUS_OK;

  for (uint8_t index_offset = 0; index_offset < token_index_size; index_offset++) {
    status = sl_token_manager_set_data(token_base + index_offset, default_token_value, token_size);
    if (status != SL_STATUS_OK) {
      break;
    }
  }
#else // EZSP_HOST
  status = sli_zigbee_host_token_init(token_base, default_token_value, token_size, token_index_size, token_is_counter);
#endif // EZSP_HOST
  return status;
}

sl_status_t sl_zigbee_initialize_basic_token(uint32_t token,
                                             void *default_token_value,
                                             uint32_t token_size)
{
  return sli_zigbee_stack_initialize_token(token, default_token_value, token_size, 1, 0);
}

sl_status_t sl_zigbee_initialize_counter_token(uint32_t token,
                                               void *default_token_value,
                                               uint32_t token_size)
{
  return sli_zigbee_stack_initialize_token(token, default_token_value, token_size, 1, 1);
}

sl_status_t sl_zigbee_initialize_index_token(uint32_t token_base,
                                             void *default_token_value,
                                             uint32_t token_size,
                                             uint8_t token_index_size)
{
  return sli_zigbee_stack_initialize_token(token_base, default_token_value, token_size, token_index_size, 0);
}

// UC simulation test default values
__attribute__((weak)) uint8_t sl_zigbee_get_zc_and_zr_count(void)
{
  return 1;
}
__attribute__((weak)) uint8_t sl_zigbee_get_supported_networks(void)
{
  return 1;
}
__attribute__((weak)) uint8_t sl_zigbee_get_key_table_size(void)
{
  return 12;
}
__attribute__((weak)) uint8_t sl_zigbee_get_child_table_size(void)
{
  return 6;
}
__attribute__((weak)) uint8_t sl_zigbee_get_certificate_table_size(void)
{
  return 1;
}
__attribute__((weak)) uint8_t sl_zigbee_get_binding_table_size(void)
{
  return 30;
}
