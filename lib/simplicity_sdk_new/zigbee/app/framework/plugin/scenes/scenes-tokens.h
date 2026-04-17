/***************************************************************************//**
 * @file
 * @brief Tokens for the Scenes plugin.
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

#include "scenes-config.h"
#include "sl_token_manager_defines.h"

#if (SL_ZIGBEE_AF_PLUGIN_SCENES_USE_TOKENS == 1)

#define CREATOR_SCENES_NUM_ENTRIES  (0x8723)
#define NVM3KEY_SCENES_NUM_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x8723)
#define COMMON_TOKEN_SCENES_NUM_ENTRIES SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x8723), 0)

#define CREATOR_SCENES_TABLE        (0x8724)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_SCENES_TABLE       (NVM3KEY_DOMAIN_ZIGBEE | 0x4080)
#define COMMON_TOKEN_SCENES_TABLE SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x4080), 0)

#define TOKEN_SCENES_NUM_ENTRIES_DEFAULT 0x00
#define TOKEN_SCENES_TABLE_DEFAULT { SL_ZIGBEE_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID }

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(SCENES_NUM_ENTRIES, uint8_t, TOKEN_SCENES_NUM_ENTRIES_DEFAULT)
DEFINE_INDEXED_TOKEN(SCENES_TABLE,
                     sl_zigbee_af_scene_table_entry_t,
                     SL_ZIGBEE_AF_PLUGIN_SCENES_TABLE_SIZE,
                     TOKEN_SCENES_TABLE_DEFAULT)
#endif //DEFINETOKENS

#endif // (SL_ZIGBEE_AF_PLUGIN_SCENES_USE_TOKENS == 1)
