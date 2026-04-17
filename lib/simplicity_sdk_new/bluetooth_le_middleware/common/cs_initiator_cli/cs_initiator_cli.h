/***************************************************************************//**
 * @file
 * @brief CS Initiator command line interface API
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

#include <stdint.h>

/***************************************************************************//**
 * CLI Getter for CS mode
 * @return mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_mode(void);

/***************************************************************************//**
 * CLI Getter for CS sub mode
 * @return sub mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_sub_mode(void);

/***************************************************************************//**
 * CLI Getter for CS algorithm mode
 * @return algo_mode
 ******************************************************************************/
uint8_t cs_initiator_cli_get_algo_mode(void);

/***************************************************************************//**
 * CLI Getter for CS channel map preset
 * @return channel_map_preset
 ******************************************************************************/
uint8_t cs_initiator_cli_get_preset(void);

/***************************************************************************//**
 * CLI Getter for Antenna Configuration Index (ACI)
 * @return antenna_configuration_idex
 ******************************************************************************/
uint8_t cs_initiator_cli_get_antenna_config_index(void);

/***************************************************************************//**
 * CLI Getter for Antenna usage for CS SYNC packets
 * @return cs_sync_antenna_usage
 ******************************************************************************/
uint8_t cs_initiator_cli_get_cs_sync_antenna_usage(void);

/***************************************************************************//**
 * CLI Getter for connection PHY
 * @return conn_phy
 ******************************************************************************/
uint8_t cs_initiator_cli_get_conn_phy(void);

/***************************************************************************//**
 * CLI Getter for number of procedures to be executed
 * @return procedure_counter
 ******************************************************************************/
uint8_t cs_initiator_cli_get_procedure_counter(void);
