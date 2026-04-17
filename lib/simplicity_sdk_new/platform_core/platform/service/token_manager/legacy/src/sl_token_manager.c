/***************************************************************************//**
 * @file sl_token_manager.c
 * @brief TOKEN_MANAGER API implementation.
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdint.h>
#include <stdbool.h>
#include "sl_token_api.h"
#include "sl_token_manager.h"
#include "sl_token_manufacturing_api.h"
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#if defined(SL_CATALOG_SIM_EEPROM2_TO_NVM3_UPGRADE_PRESENT)
#include "sl_sim_eeprom2_to_nvm3_upgrade.h"
#endif // SL_CATALOG_SIM_EEPROM2_TO_NVM3_UPGRADE_PRESENT
#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS) || defined(USE_NVM3)
#include "nvm3.h"
#endif

#if defined(SL_TOKEN_MANAGER_ENABLE_DEBUG_TRACE)
//prints debug data from the token access functions
#include "printf.h"
#define TOKENDBG(x) x
#else
#define TOKENDBG(x)
#endif

static bool tokensActive = false;

bool nvm3OverrideActive = false;

__WEAK void halNvm3Callback(sl_status_t status)
{
  // NVM3 does error handling internally, but this callback is included
  // in case the application requires any custom error handling.
  (void) status;
}

__WEAK bool sl_token_assert_on_cache_overflow_callback(void)
{
  // Return true here means assertion when the NVM3 cache overflows
  // Customers can strongly re-define this callback to fit their purpose.
  return true;
}

#if defined(SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS) || defined(USE_NVM3)
static uint32_t remapTokenToTokenNvm3KeysIndex(uint32_t token)
{
  uint32_t i = 0;

  if (token >= TOKEN_COUNT) {
    for (i = 0; (i < nvm3_countObjects(nvm3_defaultHandle)) && (i < TOKEN_COUNT); i++) {
      if (token == tokenNvm3Keys[i]) {
        TOKENDBG(printf("REMAPPING token=%0lx to index %0lx (tokenNvm3Keys[%0lx]=%0lx)\r\n", token, i, i, tokenNvm3Keys[i]); )
        return i;
      }
    }
  }
  return token;
}

/**************************************************************************//**
 * Initializes the Token Manager.
 *****************************************************************************/
sl_status_t sl_token_init(void)
{
  uint8_t i;
  uint8_t arrayIndex;
  size_t size;
  uint32_t objectType;
  sl_status_t slStatus, status;
  nvm3_HalInfo_t halInfo;

#if !defined(NVM3_OVERRIDE_ENABLE)
  //TODO: For now all the tokens are stored in NVM3.
  //Below INIT should be enabled, once we enable the flash access via SE.
  //Manufacturing init
  sl_token_mfg_init();
#endif

  slStatus = nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
  TOKENDBG(printf("halStackInitTokens nvm3_open status: 0x%0lx\r\n", slStatus); )
  if (slStatus != SL_STATUS_OK) {
    (void) nvm3_halOpen(nvm3_defaultInit->halHandle, nvm3_defaultInit->nvmAdr, nvm3_defaultInit->nvmSize);
    (void) nvm3_halGetInfo(nvm3_defaultInit->halHandle, &halInfo);
    (void) nvm3_halNvmAccess(nvm3_defaultInit->halHandle, NVM3_HAL_NVM_ACCESS_RDWR);
    for (i = 0; i < nvm3_defaultInit->nvmSize / halInfo.pageSize; i++) {
      slStatus = nvm3_halPageErase(nvm3_defaultInit->halHandle,
                                   (nvm3_HalPtr_t)((size_t) nvm3_defaultInit->nvmAdr + i * halInfo.pageSize));
      TOKENDBG(printf("nvm3_halPageErase %d status: 0x%0lx\r\n",
                      i,
                      slStatus); )
    }
    (void) nvm3_halNvmAccess(nvm3_defaultInit->halHandle, NVM3_HAL_NVM_ACCESS_NONE);
    (void) nvm3_halClose(nvm3_defaultInit->halHandle);

    slStatus = nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
    TOKENDBG(printf("halStackInitTokens second nvm3_open status: 0x%0lx\r\n",
                    slStatus); )
  }

  for (i = 0; i < (uint8_t) TOKEN_COUNT; i++) {
    if (slStatus == SL_STATUS_OK) {
      TOKENDBG(printf("Token[%d]: Key=0x%lx Array size =%d \r\n",
                      i,
                      tokenNvm3Keys[i],
                      tokenArraySize[i]); )

      if (tokenIsCnt[i]) { // Counter token
        status = nvm3_getObjectInfo(nvm3_defaultHandle,
                                    tokenNvm3Keys[i],
                                    &objectType,
                                    &size);

        // Create NVM3 objects for tokens if they don't exist already
        // or if type or size doesn't match
        if ((status == SL_STATUS_NOT_FOUND)
            || ((status == SL_STATUS_OK)
                && (objectType != NVM3_OBJECTTYPE_COUNTER))) {
          TOKENDBG(printf("Creating counter token from default value\r\n"); )

          // If object type is wrong we need to delete it before creating the
          // new one with the correct type.
          if (status == SL_STATUS_OK) {
            (void) nvm3_deleteObject(nvm3_defaultHandle, tokenNvm3Keys[i]);
            TOKENDBG(printf("Deleting object of wrong type\r\n"); )
          }

          slStatus = nvm3_writeCounter(nvm3_defaultHandle,
                                       tokenNvm3Keys[i],
                                       *(const uint32_t *) tokenDefaults[i]);
          TOKENDBG(printf("halStackInitTokens nvm3_writeCounter value: 0x%0lx status: 0x%0lx\r\n",
                          *(uint32_t *) tokenDefaults[i],
                          slStatus); )
        }
      } else { // Basic or indexed token
        // For indexed tokens we need to create every index in the indexed
        // token key domain if it doesn't exist already or if size or type doesn't match
        for (arrayIndex = 0; arrayIndex < tokenArraySize[i]; arrayIndex++) {
          status = nvm3_getObjectInfo(nvm3_defaultHandle,
                                      tokenNvm3Keys[i] + arrayIndex,
                                      &objectType,
                                      &size);

          if ((status == SL_STATUS_NOT_FOUND)
              || ((status == SL_STATUS_OK)
                  && ((objectType != NVM3_OBJECTTYPE_DATA)
                      || (size != tokenSize[i])))) {
            TOKENDBG(printf("Creating basic/index token (index: %x) from default value\r\n", arrayIndex); )

            // If the object type is wrong we need to delete it before creating the
            // new one with the correct type.
            // Wrong size will be corrected by the data write itself
            if (((status == SL_STATUS_OK)
                 && ((objectType != NVM3_OBJECTTYPE_DATA)))) {
              (void) nvm3_deleteObject(nvm3_defaultHandle, tokenNvm3Keys[i] + arrayIndex);
              TOKENDBG(printf("Deleting object of wrong type\r\n"); )
            }

            slStatus = nvm3_writeData(nvm3_defaultHandle,
                                      tokenNvm3Keys[i] + arrayIndex,
                                      (const void *) tokenDefaults[i],
                                      tokenSize[i]);
            TOKENDBG(printf("halStackInitTokens nvm3_writeData length: %d  status: 0x%0lx\r\n",
                            tokenSize[i],
                            slStatus); )
            if (slStatus != SL_STATUS_OK) {
              break;
            }
          }
        }

        if (tokenIsIdx[i]) {
          // We need to delete objects for any indexes that are no longer used.
          for (arrayIndex = tokenArraySize[i]; arrayIndex < 0x7FU; arrayIndex++) {
            status = nvm3_getObjectInfo(nvm3_defaultHandle,
                                        tokenNvm3Keys[i] + arrayIndex,
                                        &objectType,
                                        &size);

            if (status == SL_STATUS_NOT_FOUND) {
              // If we don't find an object we assume there are no objects at higher indexes
              // and stop looking.
              break;
            } else {
              slStatus = nvm3_deleteObject(nvm3_defaultHandle, tokenNvm3Keys[i] + arrayIndex);
              TOKENDBG(printf("halStackInitTokens Erased unused index %d of token with key 0x%0lx\r\n",
                              arrayIndex,
                              tokenNvm3Keys[i]); )
              if (slStatus != SL_STATUS_OK) {
                break;
              }
            }
          }
        }
      }
    }
  }

  tokensActive = slStatus == SL_STATUS_OK;

  // If the NVM3 cache overflows it is too small to index all live and deleted NVM3 objects
  if (nvm3_defaultHandle->cache.overflow == true) {
    TOKENDBG(printf("NVM3 Cache overflowed during token initialization, please increase the cache size! \r\n"); )
    assert(!sl_token_assert_on_cache_overflow_callback());
  }
#if defined(NVM3_OVERRIDE_ENABLE)
  if (tokensActive) {
    nvm3OverrideActive = true;
  }
#endif
  TOKENDBG(printf("\nnumKeys=%d\r\n\r\n", nvm3_countObjects(nvm3_defaultHandle)); )
  return slStatus;
}

/***************************************************************************//**
 * Read the data stored in the specified data or manufacturing token.
 ******************************************************************************/
sl_status_t sl_token_get_data(uint32_t token,
                              uint32_t index,
                              void *data,
                              uint32_t length)
{
  uint8_t offset;
  sl_status_t status = SL_STATUS_OK;

  if ((nvm3OverrideActive == true)
      && ((((token & NVM3KEY_MFG_TOKEN_RANGE_MASK) >> NVM3KEY_MFG_TOKEN_RANGE_ID_SHIFT) == NVM3KEY_MFG_UD_TOKEN_RANGE_ID)
          || (((token & NVM3KEY_MFG_TOKEN_RANGE_MASK) >> NVM3KEY_MFG_TOKEN_RANGE_ID_SHIFT) == NVM3KEY_MFG_LB_TOKEN_RANGE_ID))) {
    //Its an override request
    status = nvm3_readPartialData(nvm3_defaultHandle,
                                  token,
                                  data,
                                  index,
                                  length);
  } else {
    TOKENDBG(printf("Get token[%0lx]: Key=0x%0lx Index= 0x%0lx  length= 0x%0lx\r\n",
                    token,
                    tokenNvm3Keys[token],
                    index,
                    length); )

    token = remapTokenToTokenNvm3KeysIndex(token);

    if ( token < TOKEN_COUNT) {
      if (tokensActive) {
        if (tokenIsCnt[token]) {
          status = nvm3_readCounter(nvm3_defaultHandle,
                                    tokenNvm3Keys[token],
                                    data);
          TOKENDBG(printf("halInternalGetTokenData nvm3_readCounter value: 0x%0lx status: 0x%0lx\r\n",
                          *(uint32_t *) data,
                          status); )
        } else {       // Basic/indexed token
          offset = index == 0x7FU ? 0U : index;
          status = nvm3_readData(nvm3_defaultHandle,
                                 tokenNvm3Keys[token] + offset,
                                 data,
                                 length);
          TOKENDBG(printf("halInternalGetTokenData nvm3_readData length: %0lx status: 0x%0lx\r\n",
                          length,
                          status); )
        }
      } else {
        TOKENDBG(printf("halInternalGetTokenData supressed\r\n"); )
      }
    } else if (token >= 256U) {
#ifdef SL_ZIGBEE_TEST
      assert(false);
#else //SL_ZIGBEE_TEST
      halInternalGetMfgTokenData(data, token, index, length);
#endif // SL_ZIGBEE_TEST
    } else {
      assert(false);
    }
  }

  if (status != SL_STATUS_OK) {
    halNvm3Callback(status);
  }

  return status;
}

/***************************************************************************//**
 * Writes the data to the specified token.
 ******************************************************************************/
sl_status_t sl_token_set_data(uint32_t token,
                              uint32_t index,
                              void *data,
                              uint32_t length)
{
  uint8_t offset;
  sl_status_t status = SL_STATUS_OK;
  uint32_t nvm3Key = 0;

  TOKENDBG(printf("Set token[%0lx]: tokenNvm3Keys[token]=0x%0lx Index= 0x%0lx  length= 0x%0lx\r\n",
                  token,
                  tokenNvm3Keys[token],
                  index,
                  length); )

  if ((nvm3OverrideActive == true)
      && ((((token & NVM3KEY_MFG_TOKEN_RANGE_MASK) >> NVM3KEY_MFG_TOKEN_RANGE_ID_SHIFT) == NVM3KEY_MFG_UD_TOKEN_RANGE_ID)
          || (((token & NVM3KEY_MFG_TOKEN_RANGE_MASK) >> NVM3KEY_MFG_TOKEN_RANGE_ID_SHIFT) == NVM3KEY_MFG_LB_TOKEN_RANGE_ID))) {
    //Its an override request
    nvm3Key = token;
  } else {
    token = remapTokenToTokenNvm3KeysIndex(token);

    if (token < TOKEN_COUNT) {
      // token is actually a NVM3 token, get the key from the list
      nvm3Key = tokenNvm3Keys[token];
    } else if (token >= 256U) {
#ifdef EMBER_TEST
      assert(false);
#else //EMBER_TEST
      // In case user sent an actual manufacturing token
      halInternalSetMfgTokenData(token, data, length);
      return status;
#endif // EMBER_TEST
    } else {
      // should not come here
      assert(false);
    }
  }

  if (tokensActive) {
    if ((token < TOKEN_COUNT) && (tokenIsCnt[token])) {
      status = nvm3_writeCounter(nvm3_defaultHandle,
                                 nvm3Key,
                                 *(uint32_t *) data);
      TOKENDBG(printf("halInternalSetTokenData nvm3_writeCounter value: 0x%0lx status: 0x%0lx\r\n",
                      *(uint32_t *) data,
                      status); )
    } else {   //Basic/indexed token
      offset = index == 0x7FU ? 0U : index;
      status = nvm3_writeData(nvm3_defaultHandle,
                              nvm3Key + offset,
                              data,
                              length);
      TOKENDBG(printf("halInternalSetTokenData nvm3_writeData length: %0lx status: 0x%0lx\r\n",
                      length,
                      status); )
    }
  } else {
    TOKENDBG(printf("halInternalSetTokenData supressed\r\n"); )
  }

  if (status != SL_STATUS_OK) {
    halNvm3Callback(status);
  }

  return status;
}

/***************************************************************************//**
 * Increments the value of a counter token.  This call does not support
 * manufacturing tokens.
 ******************************************************************************/
sl_status_t sl_token_increment_counter(uint32_t token)
{
  sl_status_t status = SL_STATUS_OK;

  token = remapTokenToTokenNvm3KeysIndex(token);

  if (tokensActive) {
    TOKENDBG(printf("Increment token[%0lx]: tokenNvm3Keys[token]=0x%0lx\r\n",
                    token,
                    tokenNvm3Keys[token]); )
    status = nvm3_incrementCounter(nvm3_defaultHandle, tokenNvm3Keys[token], NULL);
    TOKENDBG(printf("halInternalIncrementCounterToken nvm3_incrementCounter status: 0x%0lx\r\n",
                    status); )
  } else {
    TOKENDBG(printf("halInternalIncrementCounterToken supressed\r\n"); )
  }
  if (status != SL_STATUS_OK) {
    halNvm3Callback(status);
  }

  return status;
}

/***************************************************************************//**
 * This call support deleting manufacturing tokens from NVM3 region.
 ******************************************************************************/
sl_status_t sl_token_delete_token(uint32_t token)
{
  sl_status_t status = SL_STATUS_OK;

  if (nvm3OverrideActive == false) {
    //NVM3 override is not activated
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = nvm3_deleteObject(nvm3_defaultHandle, SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token));
  TOKENDBG(printf("sl_token_delete_mfg_token deleted token: %0lx status: 0x%0lx\r\n",
                  token,
                  status); )

  return status;
}
#endif // SL_COMMON_TOKEN_MANAGER_ENABLE_DYNAMIC_TOKENS || USE_NVM3
