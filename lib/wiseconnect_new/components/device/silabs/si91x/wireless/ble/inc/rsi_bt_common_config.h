/*******************************************************************************
 * @file  rsi_bt_common_config.h
 * @brief : This file contains user configurable details to configure the device
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

#ifndef RSI_BT_COMMON_CONFIG_H
#define RSI_BT_COMMON_CONFIG_H
/** @addtogroup BT_BLE_CONSTANTS
 *  @{
 */

#ifndef BD_ADDR_ARRAY_LEN
#define BD_ADDR_ARRAY_LEN 18 ///< length of the Bluetooth device address array
#endif
#ifndef RSI_LINKKEY_REPLY_SIZE
#define RSI_LINKKEY_REPLY_SIZE 16 ///< size of the link key reply
#endif
// WLAN + BT Classic coex mode
#ifndef RSI_OPERMODE_WLAN_BT_CLASSIC
#define RSI_OPERMODE_WLAN_BT_CLASSIC 5 ///< operation mode for WLAN and Bluetooth Classic
#endif
#ifndef RSI_BT_BLE_DUAL_MODE
#define RSI_BT_BLE_DUAL_MODE 9 ///< operation mode for Bluetooth and BLE dual mode
#endif

#ifndef BT_GLOBAL_BUFF_LEN
#define BT_GLOBAL_BUFF_LEN 10000 ///< size of the global buffer for Bluetooth operations
#endif
/*=======================================================================*/
// Discovery command parameters
/*=======================================================================*/

// Discovery mode
// Start the discoverable mode
#ifndef START_DISCOVERY
#define START_DISCOVERY 0x01 ///< command to start device discovery
#endif

// Start the limited discoverable mode
#ifndef START_LIMITED_DISCOVERY
#define START_LIMITED_DISCOVERY 0x02 ///< command to start limited discovery mode
#endif

// Stop the discoverable mode
#ifndef STOP_DISCOVERY
#define STOP_DISCOVERY 0x00 ///< command to stop discovery mode
#endif
/*=======================================================================*/
// Connectability command parameters
/*=======================================================================*/

// Connectable mode
// Start the connectable mode
#ifndef CONNECTABLE
#define CONNECTABLE 0x01 ///< state indicating the device is connectable
#endif

// Stop the connectable mode
#ifndef NON_CONNECTABLE
#define NON_CONNECTABLE 0x00 ///< state indicating the device is non-connectable
#endif

/*=======================================================================*/
//  SSP Confirmation command parameters
/*=======================================================================*/
// Confimation type
// Accept confirmation
#ifndef ACCEPT
#define ACCEPT 0x01 ///< SSP confirmation command parameter indicating acceptance
#endif

// Reject confirmation
#ifndef REJECT
#define REJECT 0x00 ///< SSP confirmation command parameter indicating rejection
#endif
/*=======================================================================*/
//  BT classic profile enable BITS
/*=======================================================================*/
#ifndef RSI_SPP_PROFILE_BIT
#define RSI_SPP_PROFILE_BIT BIT(0) ///< bitmask to enable the SPP profile in Bluetooth Classic (BIT(0))
#endif

#ifndef RSI_A2DP_PROFILE_BIT
#define RSI_A2DP_PROFILE_BIT BIT(1) ///< bitmask to enable the A2DP profile in Bluetooth Classic (BIT(1))
#endif

#ifndef RSI_AVRCP_PROFILE_BIT
#define RSI_AVRCP_PROFILE_BIT BIT(2) ///< bitmask to enable the AVRCP profile in Bluetooth Classic (BIT(2))
#endif

#ifndef RSI_HFP_PROFILE_BIT
#define RSI_HFP_PROFILE_BIT BIT(3) ///< bitmask to enable the HFP profile in Bluetooth Classic (BIT(3))
#endif

#ifndef RSI_PBAP_PROFILE_BIT
#define RSI_PBAP_PROFILE_BIT \
  BIT(4) #define RSI_PBAP_PROFILE_BIT BIT(4) ///< bitmask to enable the PBAP profile in Bluetooth Classic (BIT(4))
#endif
/*=======================================================================*/
//  A2DP Profile Related
/*=======================================================================*/

#ifndef A2DP_BURST_MODE
#define A2DP_BURST_MODE 1 ///< A2DP profile setting for burst mode ,  0 - Disable, 1 - Enable
#endif

#ifndef A2DP_BURST_SIZE
#define A2DP_BURST_SIZE 1 ///<  Number of PCM/MP3 packets buffered
#endif

#ifndef PCM_AUDIO
#define PCM_AUDIO 1 ///< Audio mode using PCM (Pulse Code Modulation)
#endif
#ifndef SBC_AUDIO
#define SBC_AUDIO 2 ///< Audio mode using SBC (Subband Coding)
#endif
#ifndef MP3_AUDIO
#define MP3_AUDIO 3 ///< Audio mode using MP3 (MPEG-1 Audio Layer 3)
#endif

#ifndef RSI_AUDIO_DATA_TYPE
#define RSI_AUDIO_DATA_TYPE \
  SBC_AUDIO ///< Defines the audio data type for the A2DP profile as SBC (Subband Coding) (Value = SBC_AUDIO)
#endif
#ifndef PCM_INPUT_BUFFER_SIZE
#define PCM_INPUT_BUFFER_SIZE (30 * 512) ///< Size of the PCM input buffer in bytes
#endif

#ifndef MP3_INPUT_BUFFER_SIZE
#define MP3_INPUT_BUFFER_SIZE (10 * 512) ///< Size of the MP3 input buffer in bytes
#endif

#ifndef BIN_FILE
#define BIN_FILE 1 ///< File type indicating binary format
#endif
#ifndef BT_ARRAY
#define BT_ARRAY 2 ///< Array type for Bluetooth data
#endif
#ifndef SD_BIN_FILE
#define SD_BIN_FILE 3 ///< File type indicating a binary format for SD
#endif

#ifndef RSI_AUDIO_DATA_SRC
#define RSI_AUDIO_DATA_SRC BIN_FILE ///< Defines the audio data source as a binary file (Value = BIN_FILE)
#endif

#ifndef PRE_ENC_BUF_LEN
#define PRE_ENC_BUF_LEN 8 ///< Length of the pre-encoded buffer (Value = 8)
#endif

//avdtp related defines
#ifndef ACCEPTOR_ROLE
#define ACCEPTOR_ROLE 1 ///< AVDTP role indicating the acceptor (Value = 1)
#endif
#ifndef INITIATOR_ROLE
#define INITIATOR_ROLE 2 ///< AVDTP role indicating the initiator (Value = 2)
#endif
#ifndef ANY_ROLE
#define ANY_ROLE 3 ///< AVDTP role indicating any role
#endif

#ifndef RSI_BT_MAX_PAYLOAD_SIZE
#define RSI_BT_MAX_PAYLOAD_SIZE \
  1040 ///< BT MTU size changes //310 Max supported is 200, but 190 gives optimum Tx throughput
#endif

#ifndef BT_BDR_MODE
#define BT_BDR_MODE 0 ///< 1 - HP chain
#endif

#ifndef USE_REM_MTU_SIZE_ONLY
#define USE_REM_MTU_SIZE_ONLY 1 ///< Flag to use only the remote MTU size
#endif

#ifndef TA_BASED_ENCODER
#define TA_BASED_ENCODER 0 ///< Encoder type based on TA
#endif

#ifdef __ZEPHYR__
#ifndef RSI_BLE_SET_RAND_ADDR
#define RSI_BLE_SET_RAND_ADDR "00:23:A7:12:34:56"
#endif

#ifndef RSI_BLE_MAX_NBR_PERIPHERALS
#define RSI_BLE_MAX_NBR_PERIPHERALS 3
#endif

#ifndef RSI_BLE_MAX_NBR_CENTRALS
#define RSI_BLE_MAX_NBR_CENTRALS 1
#endif
#endif

/** @} */
#endif //RSI_BT_COMMON_CONFIG_H
