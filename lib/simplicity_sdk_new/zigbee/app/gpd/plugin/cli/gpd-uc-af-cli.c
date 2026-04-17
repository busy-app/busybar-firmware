/***************************************************************************//**
 * @file app_cli.c
 * @brief app_cli.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "sl_cli.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif
#include "gpd-components-common.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#define gpdDebugPrintf sl_zigbee_core_debug_print
#else
#define gpdDebugPrintf(...)
#endif
#define LENGTH_ARG_FOR_GPD_SRC_ID (sizeof(uint8_t) + sizeof(uint32_t))
#define LENGTH_ARG_FOR_GPD_IEEE_ID (sizeof(uint8_t) + sizeof(uint8_t) + SL_ZIGBEE_GPD_EUI64_SIZE)
#define GPD_KEY_SIZE 16
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
// following function is part of the application implementation.
extern void sl_zigbee_gpd_app_single_event_commission(void);
/******************************************************************************
 * CLI - info message: Unique ID of the board
 *****************************************************************************/
void sl_zigbee_gpd_af_cli_reset_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  NVIC_SystemReset();
}

void sl_zigbee_gpd_af_cli_node_info(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  if (gpd->addr.appId == SL_ZIGBEE_GPD_APP_ID_SRC_ID) {
    gpdDebugPrintf("App Id = %d SrcId = 0x%4lx\n", gpd->addr.appId, gpd->addr.id.srcId);
  } else if (gpd->addr.appId == SL_ZIGBEE_GPD_APP_ID_IEEE_ID) {
    uint8_t *ieee = gpd->addr.id.ieee;
    gpdDebugPrintf("App Id = %d Ep = %d IEEE=0x%02X%02X%02X%02X%02X%02X%02X%02X\n",
                   gpd->addr.appId, gpd->addr.gpdEndpoint,
                   ieee[7], ieee[6], ieee[5], ieee[4], ieee[3], ieee[2], ieee[1], ieee[0]);
  }

  gpdDebugPrintf("Auto-Comm=%d\n", gpd->autoCommissioning);
  gpdDebugPrintf("Channel=%d\n", gpd->channel);
  gpdDebugPrintf("GPD State=%d\n", gpd->gpdState);
  gpdDebugPrintf("GPD Bidir=%d\n", gpd->rxAfterTx);
  gpdDebugPrintf("  GPD rxWindow=%d msec\n", gpd->minRxWindow);
  gpdDebugPrintf("  GPD rxOffset=%d msec\n", gpd->rxOffset);
  gpdDebugPrintf("Sec Level=%d\n", gpd->securityLevel);
  gpdDebugPrintf("  Sec KeyType=%d\n", gpd->securityKeyType);
  gpdDebugPrintf("  Sec Key={%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X} \n",
                 gpd->securityKey[0], gpd->securityKey[1], gpd->securityKey[2], gpd->securityKey[3],
                 gpd->securityKey[4], gpd->securityKey[5], gpd->securityKey[6], gpd->securityKey[7],
                 gpd->securityKey[8], gpd->securityKey[9], gpd->securityKey[10], gpd->securityKey[11],
                 gpd->securityKey[12], gpd->securityKey[13], gpd->securityKey[14], gpd->securityKey[15]);
  gpdDebugPrintf("  Sec FC=%ld\n", gpd->securityFrameCounter);
}

void sl_zigbee_gpd_af_cli_channel_req(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
}

void sl_zigbee_gpd_af_cli_comm_req(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  uint8_t action = sl_cli_get_argument_uint8(arguments, 0);
  if (action == 1) {
    sl_zigbee_gpd_af_plugin_commission(gpd);
  } else if (action == 0) {
    sl_zigbee_gpd_af_plugin_de_commission(gpd);
  } else if (action == 0xff) {
    sl_zigbee_gpd_app_single_event_commission();
  }
}

void sl_zigbee_gpd_af_cli_node_set_addr(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  unsigned int argLength = 0;
  uint8_t *str = sl_cli_get_argument_hex(arguments, 0, &argLength);
  // Process only if the length is exactly as needed
  if (argLength == LENGTH_ARG_FOR_GPD_SRC_ID
      || argLength == LENGTH_ARG_FOR_GPD_IEEE_ID) {
    gpd->addr.appId = str[0];
    if (gpd->addr.appId == SL_ZIGBEE_GPD_APP_ID_SRC_ID) {
      gpd->addr.id.srcId = ((uint32_t)str[1] << 24)   \
                           + ((uint32_t)str[2] << 16) \
                           + ((uint32_t)str[3] << 8)  \
                           + str[4];
    } else if (gpd->addr.appId == SL_ZIGBEE_GPD_APP_ID_IEEE_ID) {
      // Endpoint
      gpd->addr.gpdEndpoint = str[1];
      // A simple way to use the command for testing
      memcpy(gpd->addr.id.ieee, &str[2], SL_ZIGBEE_GPD_EUI64_SIZE);
    } else {
      // Error in the input argument
      gpdDebugPrintf("Must be exactly %d or %d bytes hex\n", \
                     LENGTH_ARG_FOR_GPD_SRC_ID, LENGTH_ARG_FOR_GPD_IEEE_ID);
    }
  }
}

void sl_zigbee_gpd_af_cli_node_set_auto_comm(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  gpd->autoCommissioning = (bool)sl_cli_get_argument_uint8(arguments, 0);
}

void sl_zigbee_gpd_af_cli_node_set_bi_dir_param(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  gpd->rxAfterTx = (bool)sl_cli_get_argument_uint8(arguments, 0);
  if (gpd->rxAfterTx) {
    gpd->rxOffset = sl_cli_get_argument_uint8(arguments, 1);
    gpd->minRxWindow = sl_cli_get_argument_uint8(arguments, 2);
  }
}

void sl_zigbee_gpd_af_cli_node_set_channel(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  gpd->channel = sl_cli_get_argument_uint8(arguments, 0);;
}

void sl_zigbee_gpd_af_cli_node_set_sec_key_type(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  gpd->securityKeyType = sl_cli_get_argument_uint8(arguments, 0);
}

void sl_zigbee_gpd_af_cli_node_set_sec_key(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  unsigned int argLength;
  uint8_t *key = sl_cli_get_argument_hex(arguments, 0, &argLength);
  if (argLength == GPD_KEY_SIZE) {
    (void) memcpy(gpd->securityKey, key, GPD_KEY_SIZE);
  } else {
    // Error in the input argument
    gpdDebugPrintf("Key must be exactly %d bytes hex\n", GPD_KEY_SIZE);
  }
}

void sl_zigbee_gpd_af_cli_node_set_sec_fc(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  gpd->securityFrameCounter = (uint32_t)sl_cli_get_argument_uint32(arguments, 0);;
}

void sl_zigbee_gpd_af_cli_node_set_sec_level(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  gpd->securityLevel = sl_cli_get_argument_uint8(arguments, 0);
}

// Sends a GPDF using the gpd node instance
// <frame type> <command with payload>
void sl_zigbee_gpd_af_cli_send_gpdf(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  uint8_t frameType = sl_cli_get_argument_uint8(arguments, 0);
  unsigned int cmdPayloadLength;
  uint8_t *cmdPayload = sl_cli_get_argument_hex(arguments, 1, &cmdPayloadLength);
  sl_zigbee_af_gpdf_send(frameType, gpd, cmdPayload, cmdPayloadLength, 1);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
