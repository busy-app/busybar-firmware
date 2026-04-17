/***************************************************************************//**
 * @file sl_rail_sdk_packet_assistant.h
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

#ifndef SL_RAIL_SDK_PACKET_ASSISTANT_H
#define SL_RAIL_SDK_PACKET_ASSISTANT_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_component_catalog.h"
#include "sl_rail_sdk_packet_asm.h"
#include "sl_rail_util_init_inst0_config.h"
#include "sl_rail_util_protocol_types.h"
#include "sl_rail.h"

#if defined(SL_CATALOG_APP_LOG_PRESENT)
#include "app_log.h"
#endif
#if defined(SL_CATALOG_APP_ASSERT_PRESENT)
#include "app_assert.h"
#endif

#include "rail_config.h"

/**
 * \addtogroup rail_sdk_utility
 * @{
 */
/**
 * \addtogroup rail_sdk_packet_assistant
 * @{
 */

/**************************************************************************//**
 * @addtogroup rail_sdk_packet_assistant_types Type definitions
 * @ingroup rail_sdk_packet_assistant
 * @{
 *****************************************************************************/
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
/**
 * @enum RAIL_SDK_Protocol_t
 * @brief Enumeration of different radio protocols.
 *
 * This enumeration defines the various radio protocols that can be used.
 */
typedef enum {
  CUSTOM_AND_SUN_OQPSK = 0, /*!< Custom and SUN OQPSK protocol */
  EMBER_PHY = 1, /*!< Ember PHY protocol */
  THREAD = 2, /*!< Thread protocol */
  BLE = 3, /*!< Bluetooth Low Energy protocol */
  CONNECT = 4, /*!< Connect protocol, original, SUN-FSK and OFDM */
  ZIGBEE = 5, /*!< Zigbee protocol */
  ZWAVE = 6, /*!< Z-Wave protocol */
  WISUN = 7, /*!< Wi-SUN protocol, FAN 1.0 and FAN 1.1 with OFDM */
  BPSK = 8, /*!< BPSK protocol */
  SIDEWALK = 9, /*!< Sidewalk protocol */
  LONGRANGE = 10, /*!< Long-range protocol */
  MBUS = 11, /*!< Wireless M-Bus protocol */
  SIGFOX = 12, /*!< Sigfox protocol */
  UNDEFINED = 13 /*!< Undefined protocol */
} RAIL_SDK_Protocol_t;

/** @} (end rail_sdk_packet_assistant_types) */
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * @brief Checks PHY setting to avoid errors at packet sending.
 */
void validation_check(void);

/**
 * @brief Updates assistant pointers with a new PHY index.
 * @param[in] new_phy_index The new PHY index to update.
 */
void update_assistant_pointers(uint8_t new_phy_index);

/**
 * @brief Unpacks the received packet, points to the payload, and returns the length.
 *
 * @param[in] rail_handle The RAIL handle used for unpacking the packet.
 * @param[out] rx_destination Where should the full packet be unpacked.
 * @param[out] packet_information Where should all the information of the packet stored.
 * @param[out] start_of_payload Pointer where the payload starts.
 * @return The length of the received payload.
 */
uint16_t unpack_packet(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);

/**
 * @brief Prepares the packet for sending and load it in the RAIL TX FIFO
 *
 * @param[in] rail_handle Which rail handlers should be used for the TX FIFO writing
 * @param[in] out_data The payload buffer
 * @param[in] length The length of the payload
 */
void prepare_packet(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);

/**
 * @brief Get the print packet information.
 * This function retrieves the current print packet information setting.
 * @return The current print packet information setting.
 */
uint8_t get_print_packet_info(void);

/**
 * @brief Set the print packet information.
 * This function sets a new value for the print packet information setting.
 * @param[in] new_print_packet_info The new print packet information setting to be applied.
 */
void set_print_packet_info(uint8_t new_print_packet_info);

/**
 * @brief Prints out the received packet.
 * This function prints the contents of the received packet.
 * @param[in] rx_buffer The buffer where the packet is stored.
 * @param[in] length The length of the packet.
 */
void printf_rx_packet(const uint8_t * const rx_buffer, uint16_t length);

/**
 * @brief Get the Wi-SUN OFDM rate.
 * Returns the current Wi-SUN OFDM rate.
 * @return The Wi-SUN OFDM rate.
 */
uint8_t get_wisun_ofdm_rate(void);

/**
 * @brief Set the Wi-SUN OFDM rate.
 * Sets the Wi-SUN OFDM rate to the specified value.
 * @param[in] new_rate The new Wi-SUN OFDM rate to set.
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_wisun_ofdm_rate(uint8_t new_rate);

/**
 * @brief Get the Wi-SUN OFDM scrambler.
 * Returns the current Wi-SUN OFDM scrambler.
 * @return The Wi-SUN OFDM scrambler.
 */
uint8_t get_wisun_ofdm_scrambler(void);

/**
 * @brief Set the Wi-SUN OFDM scrambler.
 * Sets the Wi-SUN OFDM scrambler to the specified value.
 * @param[in] new_scrambler The new Wi-SUN OFDM scrambler to set, 2 bits wide (0-3)
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_wisun_ofdm_scrambler(uint8_t new_scrambler);

/**
 * @brief Get the Wi-SUN FSK FCS value.
 * Returns the current Wi-SUN FSK FCS value.
 * @return The Wi-SUN FSK FCS value.
 */
uint8_t get_wisun_fsk_fcs(void);

/**
 * @brief Set the Wi-SUN FSK FCS value.
 * Sets the Wi-SUN FSK FCS value to the specified new value.
 * @param[in] new_fcs The new Wi-SUN FSK FCS value to set.
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_wisun_fsk_fcs(uint8_t new_fcs);

/**
 * @brief Get the Wi-SUN FSK Whitening value.
 * Returns the current Wi-SUN FSK Whitening value.
 * @return The Wi-SUN FSK Whitening value.
 */
uint8_t get_wisun_fsk_whitening(void);

/**
 * @brief Set the Wi-SUN FSK Whitening value.
 * Sets the Wi-SUN FSK Whitening value to the specified new value.
 * @param[in] new_whitening The new Wi-SUN FSK Whitening value to set.
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_wisun_fsk_whitening(uint8_t new_whitening);

/**
 * @brief Get the Sun OQPSK Spreading Mode.
 * Retrieves the current Sun OQPSK Spreading Mode.
 * @return The current Sun OQPSK Spreading Mode.
 */
uint8_t get_sun_oqpsk_spreading_mode(void);

/**
 * @brief Set the Sun OQPSK Spreading Mode.
 * Sets the Sun OQPSK Spreading Mode to the specified new mode.
 * @param[in] new_spreading_mode The new Sun OQPSK Spreading Mode to set.
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_sun_oqpsk_spreading_mode(uint8_t new_spreading_mode);

/**
 * @brief Get the Sun OQPSK Rate Mode.
 * Retrieves the current Sun OQPSK Rate Mode.
 * @return The current Sun OQPSK Rate Mode.
 */
uint8_t get_sun_oqpsk_rate_mode(void);

/**
 * @brief Set the Sun OQPSK Rate Mode.
 * Sets the Sun OQPSK Rate Mode to the specified new mode.
 * @param[in] new_rate_mode The new Sun OQPSK Rate Mode to set, 2 bits wide (0-3)
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_sun_oqpsk_rate_mode(uint8_t new_rate_mode);

/**
 * @brief Get the Sidewalk FSK FCS value.
 * Retrieves the current Sidewalk FSK FCS value.
 * @return The current Sidewalk FSK FCS value.
 */
uint8_t get_sidewalk_fcs_type(void);

/**
 * @brief Set the Sidewalk FSK FCS value.
 * Sets the Sidewalk FSK FCS value to the specified new value.
 * @param[in] new_fcs The new Sidewalk FSK FCS value to set.
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_sidewalk_fcs_type(uint8_t new_fcs);

/**
 * @brief Get the Sidewalk Whitening value.
 * Retrieves the current Sidewalk Whitening value.
 * @return The current Sidewalk Whitening value.
 */
uint8_t get_sidewalk_whitening(void);

/**
 * @brief Set the Sidewalk Whitening value.
 * Sets the Sidewalk Whitening value to the specified new value.
 * @param[in] new_whitening The new Sidewalk Whitening value to set.
 * @return 1 if the value was set successfully, 0 otherwise.
 */
uint8_t set_sidewalk_whitening(uint8_t new_whitening);

#endif // SL_RAIL_SDK_PACKET_ASSISTANT_H
/** @} */ // end of packet_assistant group
/** @} */ // end of utility group
