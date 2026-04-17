/***************************************************************************//**
 * @file
 * @brief Token defines for the Green Power Translation Table plugin.
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
#include "green-power-translation-table-config.h"

// Green Power Translation Table Tokens
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1)

#define CREATOR_TRANSLATION_TABLE     (0x8729)
#define CREATOR_CUSTOMIZED_TABLE      (0x872A)
#define CREATOR_ADDITIONALINFO_TABLE  (0x872B)
#define CREATOR_ADDITIONALINFO_TABLE_VALID_ENTRIES  (0x872C)
#define CREATOR_TRANSLATION_TABLE_TOTAL_ENTRIES     (0x872D)
#define CREATOR_ADDITIONALINFO_TABLE_TOTAL_ENTRIES  (0x872E)

// NVM3 indexed tokens need special consideration for key number assigment
// Ref :
//     Section 3.1.2  Special Considerations for Indexed Tokens
//  of an1135-using-third-generation-nonvolatile-memory.pdf

// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_TRANSLATION_TABLE    (NVM3KEY_DOMAIN_ZIGBEE | 0x8729)
#define COMMON_TOKEN_TRANSLATION_TABLE SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x8729), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_CUSTOMIZED_TABLE     (NVM3KEY_DOMAIN_ZIGBEE | 0x87A9)
#define COMMON_TOKEN_CUSTOMIZED_TABLE SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x87A9), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ADDITIONALINFO_TABLE (NVM3KEY_DOMAIN_ZIGBEE | 0x8829)
#define COMMON_TOKEN_ADDITIONALINFO_TABLE SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x8829), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_ADDITIONALINFO_TABLE_VALID_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x88A9)
#define COMMON_TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x88A9), 0)

// Basic Tokens
#define NVM3KEY_TRANSLATION_TABLE_TOTAL_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x8929)
#define NVM3KEY_ADDITIONALINFO_TABLE_TOTAL_ENTRIES (NVM3KEY_DOMAIN_ZIGBEE | 0x892A)
#define COMMON_TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x8929), 0)
#define COMMON_TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x892A), 0)

#define TOKEN_TRANSLATION_TABLE_DEFAULT { 0x00, 0x00, 0x00, 0x00, 0x00, { .id.sourceId = 0x00000000, 0x00, 0x00 }, 0x00, 0x00 }
#define TOKEN_CUSTOMIZED_TABLE_DEFAULT { 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x00, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
#define TOKEN_ADDITIONALINFO_TABLE_DEFAULT { 0x00, 0x00, .optionData.compactAttr = { 0x00, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x0000 } }
#define TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES_DEFAULT { 0x00 }
#define TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES_DEFAULT 0x00
#define TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES_DEFAULT 0x00

#include "green-power-translation-table.h"

#define SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_INDEXED_TOKEN(TRANSLATION_TABLE,
                     sli_zigbee_af_gp_command_translation_table_entry,
                     SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE,
                     TOKEN_TRANSLATION_TABLE_DEFAULT)
DEFINE_INDEXED_TOKEN(CUSTOMIZED_TABLE,
                     sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t,
                     SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE,
                     TOKEN_CUSTOMIZED_TABLE_DEFAULT)
DEFINE_INDEXED_TOKEN(ADDITIONALINFO_TABLE,
                     sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t,
                     SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE,
                     TOKEN_ADDITIONALINFO_TABLE_DEFAULT)
DEFINE_INDEXED_TOKEN(ADDITIONALINFO_TABLE_VALID_ENTRIES,
                     uint8_t,
                     SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE,
                     TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES_DEFAULT)
DEFINE_BASIC_TOKEN(TRANSLATION_TABLE_TOTAL_ENTRIES, uint8_t, TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES_DEFAULT)
DEFINE_BASIC_TOKEN(ADDITIONALINFO_TABLE_TOTAL_ENTRIES, uint8_t, TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES_DEFAULT)
#endif //DEFINETOKENS
#endif //SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS
