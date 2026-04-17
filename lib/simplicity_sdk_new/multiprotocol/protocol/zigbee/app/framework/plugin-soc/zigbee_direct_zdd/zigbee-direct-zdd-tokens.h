/***************************************************************************//**
 * @file zigbee-direct-zdd-tokens.h
 * @brief Tokens for the Zigbee Direct ZDD plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "zigbee_direct_zdd_config.h"
/* custom application tokens */
#define CREATOR_PLUGIN_ZDD_AUTH_STATUS  (0x3001)
#define CREATOR_PLUGIN_ZDD_JOIN_TIMEOUT  (0x3002)

#define NVM3KEY_PLUGIN_ZDD_AUTH_STATUS (NVM3KEY_DOMAIN_ZIGBEE | 0x3001)
#define NVM3KEY_PLUGIN_ZDD_JOIN_TIMEOUT (NVM3KEY_DOMAIN_ZIGBEE | 0x3002)

#define COMMON_TOKEN_PLUGIN_ZDD_AUTH_STATUS SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x3001), 0)
#define COMMON_TOKEN_PLUGIN_ZDD_JOIN_TIMEOUT SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x3002), 0)

#define TOKEN_PLUGIN_ZDD_AUTH_STATUS_DEFAULT    0x00
#define TOKEN_PLUGIN_ZDD_JOIN_TIMEOUT_DEFAULT   SL_ZIGBEE_DIRECT_ZDD_DEFAULT_JOIN_TIMEOUT_SEC

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(PLUGIN_ZDD_AUTH_STATUS, uint8_t, TOKEN_PLUGIN_ZDD_AUTH_STATUS_DEFAULT)
DEFINE_BASIC_TOKEN(PLUGIN_ZDD_JOIN_TIMEOUT, uint32_t, TOKEN_PLUGIN_ZDD_JOIN_TIMEOUT_DEFAULT)
#endif //DEFINETOKENS
