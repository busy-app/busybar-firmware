/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SL_RAIL_SDK_PACKET_ASSISTANT_CONFIG_H
#define SL_RAIL_SDK_PACKET_ASSISTANT_CONFIG_H

/**************************************************************************//**
 * @defgroup rail_sdk_packet_assistant_defines Configurations
 * @ingroup rail_sdk_packet_assistant
 * @{
 *****************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Assistant print settings
// <o PRINT_PACKET_INFO> Enable assistant log prints
// <i> Default: 0
// <i> 1 enabled, 0 disabled
#define PRINT_PACKET_INFO      (0) ///< Enable assistant log prints

// </h> Assistant print settings

// <h> WISUN FSK header settings
// <o WISUN_FSK_FCS_TYPE> FCS is on/off
// <i> Default: 0
// <i> FCS is on/off.
#define WISUN_FSK_FCS_TYPE      (0) ///< Wi-SUN FCS is on/off

// <o WISUN_FSK_WHITENING> Whitening is on/off
// <i> Default: 1
// <i> Whitening is on/off
#define WISUN_FSK_WHITENING      (1) ///< Wi-SUN Whitening is on/off

// </h> WISUN FSK header settings

// <h> WISUN OFDM header settings
// <o WISUN_OFDM_RATE> Default Radio Configuration
// <i> Default: 6
#define WISUN_OFDM_RATE  (6) ///< Wi-SUN OFDM rate Configuration

// <o WISUN_OFDM_SCRAMBLER> 2 bits wide, The Scrambler field (S1-S0) specifies the scrambling seed
// <i> Default: 0
// <i> 2 bits wide, The Scrambler field (S1-S0) specifies the scrambling seed
#define WISUN_OFDM_SCRAMBLER      (0) ///< Wi-SUN OFDM Scrambler Configuration

// </h> WISUN OFDM header settings

// <h> SUN OQPSK header settings
// <o SUN_OQPSK_SPREADINGMODE> spreading mode
// <i> Default: 0
// <i> spreading mode
#define SUN_OQPSK_SPREADINGMODE      (0) ///< SUN OQPSK spreading mode Configuration

// <o SUN_OQPSK_RATEMODE> rate mode: 2 bits wide
// <i> Default: 1
// <i> rate mode: 2 bits wide
#define SUN_OQPSK_RATEMODE      (0) ///< SUN OQPSK rate mode Configuration

// </h> SUN OQPSK header settings

// <h> SideWalk FSK header settings
// <o  SIDEWALK_FSK_FCS_TYPE> FCS is on/off
// <i> Default: 0
// <i> FCS is on/off.
#define SIDEWALK_FSK_FCS_TYPE      (1) ///< SideWalk FSK FCS is on/off

// <o  SIDEWALK_FSK_WHITENING> Whitening is on/off
// <i> Default: 1
// <i> Whitening is on/off
#define  SIDEWALK_FSK_WHITENING      (1) ///< SideWalk FSK Whitening is on/off

// </h> SideWalk FSK header settings

// <<< end of configuration section >>>

/** @}*/

#endif // SL_RAIL_SDK_PACKET_ASSISTANT_CONFIG_H
