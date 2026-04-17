/***************************************************************************//**
 * @file
 * @brief Tokens for the Reporting plugin.
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

#include "sl_token_manager_api.h"
#include "sl_token_manager_defines.h"
#include "reporting-config.h"

#if (SL_ZIGBEE_AF_PLUGIN_REPORTING_ENABLE_EXPANDED_TABLE == 0)
#define CREATOR_REPORT_TABLE  (0x8725)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define NVM3KEY_REPORT_TABLE (NVM3KEY_DOMAIN_ZIGBEE | 0x4000)
#define COMMON_TOKEN_REPORT_TABLE SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x4000), 0)

#define TOKEN_REPORT_TABLE_DEFAULT {                  \
    SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED,       \
    SL_ZIGBEE_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID, \
    0x00, /* clusterId */                             \
    0x00, /* attributeId */                           \
    0x00, /* mask */                                  \
    0x00, /* manufacturerCode */                      \
    .data.reported = { 0, 0, 0 }                      \
}

#if defined SL_ZIGBEE_ZCL_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE
#define REPORT_TABLE_SIZE (SL_ZIGBEE_ZCL_GENERATED_REPORTING_CONFIG_DEFAULTS_TABLE_SIZE + SL_ZIGBEE_AF_PLUGIN_REPORTING_TABLE_SIZE)
#else
#define REPORT_TABLE_SIZE (SL_ZIGBEE_AF_PLUGIN_REPORTING_TABLE_SIZE)
#endif

#ifdef DEFINETOKENS
// Define the actual token storage information here

// Following is for backward compatibility.
// The default reporting will generate a table that is mandatory
// but user may still allocate some table for adding more reporting over
// the air or by cli as part of reporting plugin.

DEFINE_INDEXED_TOKEN(REPORT_TABLE,
                     sl_zigbee_af_plugin_reporting_entry_t,
                     REPORT_TABLE_SIZE,
                     TOKEN_REPORT_TABLE_DEFAULT)

#endif //DEFINETOKENS
#endif //(SL_ZIGBEE_AF_PLUGIN_REPORTING_ENABLE_EXPANDED_TABLE == 0)
