/***************************************************************************//**
 * @file
 * @brief Tokens for the IAS Zone Server plugin.
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

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#include "sl_token_manager_api.h"
#include "sl_token_manager_defines.h"
#endif

#include "config/ias-zone-server-config.h"

/**
 * Custom Application Tokens
 */
#define CREATOR_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD  (0x0020)
#define NVM3KEY_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD (NVM3KEY_DOMAIN_USER | 0x0020)
#define COMMON_TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | 0x0020), 0)

#define TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD_DEFAULT 0xFF

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD, uint8_t, TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD_DEFAULT)
#endif  // DEFINETOKENS
