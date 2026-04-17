/***************************************************************************//**
 * @file
 * @brief
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

#define CREATOR_CUSTOM 0xE700
#define NVM3KEY_CUSTOM (NVM3KEY_DOMAIN_ZIGBEE | 0xE700)
#define COMMON_TOKEN_CUSTOM SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE700), 0)

#define TOKEN_CUSTOM_DEFAULT { 0x00, 0x0000, 0x0000 }

#ifdef DEFINETYPES
typedef struct {
  uint8_t nodeType;
  uint16_t nodeId;
  uint16_t panId;
} tokTypeCustom;
#endif //DEFINETYPES

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(CUSTOM,
                   tokTypeCustom,
                   TOKEN_CUSTOM_DEFAULT)
#endif //DEFINETOKENS
