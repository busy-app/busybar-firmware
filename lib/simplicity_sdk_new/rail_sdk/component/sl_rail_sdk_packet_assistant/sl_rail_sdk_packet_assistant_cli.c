/***************************************************************************//**
 * @file sl_rail_sdk_packet_assistant_cli.c
 * @brief RAIL SDK - RAIL Packet Assistant Component
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_rail_sdk_packet_assistant.h"
#include "sl_cli.h"

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
void cli_get_print_packet_info(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Print setting is %s\n", get_print_packet_info() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - set_print_packet_info message: Set the print setting
 *****************************************************************************/
void cli_set_print_packet_info(sl_cli_command_arg_t *arguments)
{
  uint8_t new_print_settings = sl_cli_get_argument_uint8(arguments, 0);
  if (new_print_settings > 0) {
    set_print_packet_info(1U);
  } else {
    set_print_packet_info(0U);
  }
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Print setting is %s\n", get_print_packet_info() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - get_wisun_fsk_fcs message: Get the Wi-SUN FSK FCS setting
 *****************************************************************************/
void cli_get_wisun_fsk_fcs(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Wi-SUN FSK FCS setting is %s\n", get_wisun_fsk_fcs() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - set_wisun_fsk_fcs message: Set the FSK FCS setting
 *****************************************************************************/
void cli_set_wisun_fsk_fcs(sl_cli_command_arg_t *arguments)
{
  uint8_t new_fcs = sl_cli_get_argument_uint8(arguments, 0);
  if (set_wisun_fsk_fcs(new_fcs)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Wi-SUN FSK FCS setting is %s\n", get_wisun_fsk_fcs() > 0 ? "ON" : "OFF");
#endif
  }
}

/******************************************************************************
 * CLI - get_wisun_fsk_whitening message: Get the FSK Whitening setting
 *****************************************************************************/
void cli_get_wisun_fsk_whitening(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Wi-SUN FSK Whitening setting is %s\n", get_wisun_fsk_whitening() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - set_wisun_fsk_whitening message: Set the FSK Whitening setting
 *****************************************************************************/
void cli_set_wisun_fsk_whitening(sl_cli_command_arg_t *arguments)
{
  uint8_t new_whitening = sl_cli_get_argument_uint8(arguments, 0);
  if (set_wisun_fsk_whitening(new_whitening)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Wi-SUN FSK Whitening setting is %s\n", get_wisun_fsk_whitening() > 0 ? "ON" : "OFF");
#endif
  }
}

/******************************************************************************
 * CLI - get_wisun_ofdm_scrambler message: Get the Wi-SUN OFDM scrambler setting
 *****************************************************************************/
void cli_get_wisun_ofdm_scrambler(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Wi-SUN OFDM scrambler setting is 0x%X\n", get_wisun_ofdm_scrambler());
#endif
}

/******************************************************************************
 * CLI - set_wisun_ofdm_scrambler message: Set the Wi-SUN OFDM scrambler setting
 *****************************************************************************/
void cli_set_wisun_ofdm_scrambler(sl_cli_command_arg_t *arguments)
{
  uint8_t new_scrambler = sl_cli_get_argument_uint8(arguments, 0);
  if (set_wisun_ofdm_scrambler(new_scrambler)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Wi-SUN OFDM scrambler setting is 0x%X\n", get_wisun_ofdm_scrambler());
#endif
  }
}

/******************************************************************************
 * CLI - get_wisun_ofdm_rate message: Get the Wi-SUN OFDM rate setting
 *****************************************************************************/
void cli_get_wisun_ofdm_rate(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Wi-SUN OFDM rate setting is 0x%X\n", get_wisun_ofdm_rate());
#endif
}

/******************************************************************************
 * CLI - set_wisun_ofdm_rate message: Set the Wi-SUN OFDM rate setting
 *****************************************************************************/
void cli_set_wisun_ofdm_rate(sl_cli_command_arg_t *arguments)
{
  uint8_t new_rate = sl_cli_get_argument_uint8(arguments, 0);
  if (set_wisun_ofdm_rate(new_rate)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Wi-SUN OFDM rate setting is 0x%X\n", get_wisun_ofdm_rate());
#endif
  }
}

/******************************************************************************
 * CLI - get_sun_oqpsk_spreading_mode message: Get the Sun OQPSK spreading mode setting
 *****************************************************************************/
void cli_get_sun_oqpsk_spreading_mode(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Sun OQPSK spreading mode setting is %s\n", get_sun_oqpsk_spreading_mode() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - set_sun_oqpsk_spreading_mode message: Set the Sun OQPSK spreading mode setting
 *****************************************************************************/
void cli_set_sun_oqpsk_spreading_mode(sl_cli_command_arg_t *arguments)
{
  uint8_t new_spreading_mode = sl_cli_get_argument_uint8(arguments, 0);
  if (set_sun_oqpsk_spreading_mode(new_spreading_mode)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Sun OQPSK spreading mode setting is %s\n", get_sun_oqpsk_spreading_mode() > 0 ? "ON" : "OFF");
#endif
  }
}

/******************************************************************************
 * CLI - get_sun_oqpsk_rate_mode message: Get the Sun OQPSK rate mode setting
 *****************************************************************************/
void cli_get_sun_oqpsk_rate_mode(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Sun OQPSK rate mode setting is 0x%X\n", get_sun_oqpsk_rate_mode());
#endif
}

/******************************************************************************
 * CLI - set_sun_oqpsk_rate_mode message: Set the Sun OQPSK rate mode setting
 *****************************************************************************/
void cli_set_sun_oqpsk_rate_mode(sl_cli_command_arg_t *arguments)
{
  uint8_t new_rate_mode = sl_cli_get_argument_uint8(arguments, 0);
  if (set_sun_oqpsk_rate_mode(new_rate_mode)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Sun OQPSK rate mode setting is 0x%X\n", get_sun_oqpsk_rate_mode());
#endif
  }
}

/******************************************************************************
 * CLI - cli_get_sidewalk_fcs_type message: Get the Sidewalk FCS setting
 *****************************************************************************/
void cli_get_sidewalk_fcs_type(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Sidewalk FSK FCS setting is %s\n", get_sidewalk_fcs_type() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - set_sidewalk_fcs_type message: Set the Sidewalk FCS setting
 *****************************************************************************/
void cli_set_sidewalk_fcs_type(sl_cli_command_arg_t *arguments)
{
  uint8_t new_fcs = sl_cli_get_argument_uint8(arguments, 0);
  if (set_sidewalk_fcs_type(new_fcs)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Sidewalk FSK FCS setting is %s\n", get_sidewalk_fcs_type() > 0 ? "ON" : "OFF");
#endif
  }
}

/******************************************************************************
 * CLI - get_sidewalk_whitening message: Get the Sidewalk Whitening setting
 *****************************************************************************/
void cli_get_sidewalk_whitening(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
#if defined(SL_CATALOG_APP_LOG_PRESENT)
  app_log_info("Sidewalk Whitening setting is %s\n", get_sidewalk_whitening() > 0 ? "ON" : "OFF");
#endif
}

/******************************************************************************
 * CLI - set_sidewalk_whitening message: Set the Sidewalk Whitening setting
 *****************************************************************************/
void cli_set_sidewalk_whitening(sl_cli_command_arg_t *arguments)
{
  uint8_t new_whitening = sl_cli_get_argument_uint8(arguments, 0);
  if (set_sidewalk_whitening(new_whitening)) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Sidewalk Whitening setting is %s\n", get_sidewalk_whitening() > 0 ? "ON" : "OFF");
#endif
  }
}
