/***************************************************************************//**
 * @file
 * @brief Source file for RAIL antenna functionality
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef CLI_INTERFACE
#include "app_common.h"
#include "response_print.h"
#endif

#include "sl_rail.h"
#include "railapp_antenna.h"

#if defined(SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT)
  #include "sl_rail_util_ant_div.h"
#endif

extern sl_rail_handle_t railHandle;
sl_rail_antenna_config_t halAntennaConfig;

/***************************************************************************//**
 * @brief
 *   Configure Antenna Diversity (ANTDIV) selection.
 *
 * @details
 *   Provide access to set system function, where its source file changes for
 *   each platform. Allows use with existing RPC config for all platforms.
 *
 * @param[in] antSel Updates Antenna Diversity
 *            @li RAILAPP_ANTENNA_0 : ANTENNA0 is used
 *            @li RAILAPP_ANTENNA_1 : ANTENNA1 is used
 *            @li RAILAPP_ANTENNA_AUTO : ANTENNA selection is automatic/last used
 ******************************************************************************/
sl_rail_status_t RAILAPP_SetAntDiv(RAILAPP_AntennaSel_t antSel)
{
  return (sl_rail_config_rx_options(railHandle, SL_RAIL_RX_OPTION_ANTENNA_AUTO,
                                    (sl_rail_rx_options_t)antSel));
}

/***************************************************************************//**
 * @brief
 *   Select the RF Path to use for TX and RX.
 *
 * @details
 *   Provide access to set system function, where its source file changes for
 *   for each platform. Allows use with existing RPC config for all platforms.
 *
 * @param rfPath Sets the default antenna path.
 ******************************************************************************/
void RAILAPP_SetRfPath(sl_rail_antenna_sel_t rfPath)
{
#ifdef  _SILICON_LABS_32B_SERIES_2
  /* Antenna internal RF Path to use */
#if defined(SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT)
  sl_rail_util_ant_div_get_antenna_config(&halAntennaConfig);
#endif
  halAntennaConfig.default_rf_path = rfPath;
  sl_rail_config_antenna(railHandle, &halAntennaConfig);
#else//!_SILICON_LABS_32B_SERIES_2 */
  (void) rfPath;
#endif//_SILICON_LABS_32B_SERIES_2 */
}

#ifdef CLI_INTERFACE
void CI_SetRfPath(sl_cli_command_arg_t *args)
{
  if (inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    uint32_t rfPath = sl_cli_get_argument_uint32(args, 0);
    RAILAPP_SetRfPath(rfPath);
    responsePrint(sl_cli_get_command_string(args, 0), "RfPath:%d", rfPath);
  } else {
    // do nothing
  }
}
#endif
