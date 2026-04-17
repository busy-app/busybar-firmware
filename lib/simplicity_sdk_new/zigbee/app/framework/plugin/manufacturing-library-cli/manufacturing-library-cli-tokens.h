/***************************************************************************//**
 * @file
 * @brief Tokens for the Manufacturing Library CLI plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_token_manager_defines.h"

/**
 * Custom Application Tokens
 */
#define CREATOR_MFG_LIB_ENABLED  (0x000d)
#define NVM3KEY_MFG_LIB_ENABLED  (NVM3KEY_DOMAIN_USER | 0x000d)
#define COMMON_TOKEN_MFG_LIB_ENABLED SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | 0x000d), 0)

#define TOKEN_MFG_LIB_ENABLED_DEFAULT 0

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(MFG_LIB_ENABLED, uint8_t, TOKEN_MFG_LIB_ENABLED_DEFAULT)
#endif //DEFINETOKENS
