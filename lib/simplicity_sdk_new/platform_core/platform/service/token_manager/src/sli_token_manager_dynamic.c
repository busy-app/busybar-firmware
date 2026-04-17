/***************************************************************************//**
 * @file sli_token_manager_dynamic.c
 * @brief Dynamic token API implementation.
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "sli_token_manager_internal.h"
#include "sl_token_manager_defines.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#include "nvm3.h"

/**************************************************************************//**
 * Initialize the NVM3 for dynamic tokens usage.
 *****************************************************************************/
sl_status_t sli_token_manager_init_dynamic(void)
{
  sl_status_t status = SL_STATUS_FAIL;

  status = nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
  TOKENDBG(printf("sli_token_manager_dynamic_init nvm3_open status: 0x%0lx\r\n", status); )

  return status;
}

/***************************************************************************//**
 * Read the data associated with the specified NVM3 token.
 ******************************************************************************/
sl_status_t sli_token_manager_get_dynamic_data(uint32_t token,
                                               void *data,
                                               uint32_t offset,
                                               uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t nvm3key = token & NVM3_KEY_MASK;

  TOKENDBG(printf("Get token:0x%0lx length= 0x%0lx\r\n",
                  token,
                  length); )

  if (((token & SL_TOKEN_NVM3_OBJ_TYPE_BIT_MASK) >> SL_TOKEN_NVM3_OBJ_TYPE_OFFSET) == SL_TOKEN_NVM3_OBJ_TYPE_COUNTER_TOKEN) {
    status = nvm3_readCounter(nvm3_defaultHandle,
                              nvm3key,
                              data);
    TOKENDBG(printf("nvm3_readCounter value: 0x%0lx status: 0x%0lx\r\n",
                    *(uint32_t *) data,
                    status); )
  } else {
    // If the offset is 0, read the whole data
    status = nvm3_readPartialData(nvm3_defaultHandle,
                                  nvm3key,
                                  data,
                                  offset,
                                  length);
    TOKENDBG(printf("nvm3_readPartialData length: %0lx status: 0x%0lx\r\n",
                    length,
                    status); )
  }

  return status;
}

/***************************************************************************//**
 * Writes data to a NVM3 token.
 *
 * @param[in] token    The NVM3KEY for the token.
 * @param[in] data     A pointer to the data being written.
 * @param[in] length   The size of the token data in number of bytes.
 *
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_set_dynamic_data(uint32_t token,
                                               void *data,
                                               uint32_t length)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t nvm3key = token & NVM3_KEY_MASK;

  TOKENDBG(printf("Set token:0x%0lx nvm3key:0x%0lx length= 0x%0lx\r\n",
                  token,
                  nvm3key,
                  length); )

  if (((token & SL_TOKEN_NVM3_OBJ_TYPE_BIT_MASK) >> SL_TOKEN_NVM3_OBJ_TYPE_OFFSET) == SL_TOKEN_NVM3_OBJ_TYPE_COUNTER_TOKEN) {
    status = nvm3_writeCounter(nvm3_defaultHandle,
                               nvm3key,
                               *(uint32_t *) data);
    TOKENDBG(printf("nvm3_writeCounter value: 0x%0lx status: 0x%0lx\r\n",
                    *(uint32_t *) data,
                    status); )
  } else {     //Basic/indexed token
    status = nvm3_writeData(nvm3_defaultHandle,
                            nvm3key,
                            data,
                            length);
    TOKENDBG(printf("nvm3_writeData length: %0lx status: 0x%0lx\r\n",
                    length,
                    status); )
  }

  return status;
}
