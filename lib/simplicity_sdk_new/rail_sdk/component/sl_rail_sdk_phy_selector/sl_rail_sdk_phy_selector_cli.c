/***************************************************************************//**
 * @file sl_rail_sdk_phy_selector_cli.c
 * @brief RAIL SDK - RAIL PHY Selector CLI Component
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail_sdk_phy_selector.h"
#include "sl_cli.h"
#include "rail_config.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif
#ifdef SL_CATALOG_RAIL_PACKET_ASSISTANT_PRESENT
#include "sl_rail_sdk_packet_assistant.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

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
/******************************************************************************
 * CLI - get_print_packet_info message: Get the print setting
 *****************************************************************************/
void cli_get_phy(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#ifdef SL_CATALOG_APP_LOG_PRESENT
  app_log_info("Current phy index is: %d\n", get_selected_phy());
#endif
}

/******************************************************************************
 * CLI - get_print_packet_info message: Get the print setting
 *****************************************************************************/
void cli_get_phy_list(sl_cli_command_arg_t *arguments)
{
  (void) arguments;

  uint8_t rail_config_size = 0;
  for (uint8_t i = 0; i < 255U; i++) {
    if (channelConfigs[i] != NULL) {
      rail_config_size++;
    } else {
      break;
    }
  }

  for (uint8_t i = 0; i < rail_config_size; i++) {
    if (channelConfigs[i]->configs[0U].stackInfo != NULL) {
#ifdef SL_CATALOG_RAIL_PACKET_ASSISTANT_PRESENT
      switch (channelConfigs[i]->configs[0U].stackInfo[0]) {
        case CUSTOM_AND_SUN_OQPSK:
          if (channelConfigs[i]->configs[0U].stackInfo[1] == 0x60 || channelConfigs[i]->configs[0U].stackInfo[1] == 0x70) {
            app_log_info("Phy index:%d with stackInfo: SUN_OQPSK\n", i);
          } else {
            app_log_info("Phy index:%d with stackInfo: CUSTOM\n", i);
          }
          break;
        case EMBER_PHY:
          app_log_info("Phy index:%d with stackInfo: EMBER_PHY\n", i);
          break;
        case THREAD:
          app_log_info("Phy index:%d with stackInfo: THREAD\n", i);
          break;
        case BLE:
          app_log_info("Phy index:%d with stackInfo: BLE\n", i);
          break;
        case CONNECT:
          if (channelConfigs[i]->configs[0U].stackInfo[1] >= 0x20) {
            app_log_info("Phy index:%d with stackInfo: CONNECT OFDM\n", i);
          } else if (channelConfigs[i]->configs[0U].stackInfo[1] == 0x01) {
            app_log_info("Phy index:%d with stackInfo: CONNECT SUN-FSK\n", i);
          } else {
            app_log_info("Phy index:%d with stackInfo: CONNECT\n", i);
          }
          break;
        case ZIGBEE:
          app_log_info("Phy index:%d with stackInfo: ZIGBEE\n", i);
          break;
        case ZWAVE:
          app_log_info("Phy index:%d with stackInfo: ZWAVE\n", i);
          break;
        case WISUN:
          if (channelConfigs[i]->configs[0U].stackInfo[1] >= 0x20) {
            app_log_info("Phy index:%d with stackInfo: WISUN OFDM\n", i);
          } else {
            app_log_info("Phy index:%d with stackInfo: WISUN\n", i);
          }
          break;
        case BPSK:
          app_log_info("Phy index:%d with stackInfo: BPSK\n", i);
          break;
        case LONGRANGE:
          app_log_info("Phy index:%d with stackInfo: LONGRANGE\n", i);
          break;
        case MBUS:
          app_log_info("Phy index:%d with stackInfo: MBUS\n", i);
          break;
        case SIDEWALK:
          app_log_info("Phy index:%d with stackInfo: SIDEWALK\n", i);
          break;
        case SIGFOX:
          app_log_info("Phy index:%d with stackInfo: SIGFOX\n", i);
          break;
        default:
          app_log_info("Phy index:%d with stackInfo: UNDEFINED\n", i);
          break;
      }
#else
      app_log_info("Phy index:%d with stackInfo: %d\n", i, channelConfigs[i]->configs[0U].stackInfo[0]);
#endif
    }
#ifdef SL_CATALOG_APP_LOG_PRESENT
    else {
      app_log_info("Phy index:%d stackInfo is not present!\n", i);
    }
#endif
  }
}

/******************************************************************************
 * CLI - set_print_packet_info message: Get the print setting
 *****************************************************************************/
void cli_set_phy(sl_cli_command_arg_t *arguments)
{
  uint8_t new_phy = sl_cli_get_argument_uint8(arguments, 0);
#ifdef SL_CATALOG_APP_LOG_PRESENT
  uint8_t status = set_selected_phy(new_phy);
  if (status) {
    app_log_info("Not possible\n");
  } else {
    app_log_info("success\n");
  }
#else
  set_selected_phy(new_phy);
#endif
}
