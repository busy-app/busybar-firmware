/***************************************************************************//**
 * @file sl_wisun_regdb.h
 * @brief Wi-SUN PHY and channels parameters
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

#ifndef SL_WISUN_REGDB_H
#define SL_WISUN_REGDB_H

#include <stdint.h>
#include <stdbool.h>

/// Channel spacings as specified by the Wi-SUN specification.
enum {
  CHANNEL_SPACING_200   = 0x00,  ///< 200 kHz
  CHANNEL_SPACING_400   = 0x01,  ///< 400 kHz
  CHANNEL_SPACING_600   = 0x02,  ///< 600 kHz
  CHANNEL_SPACING_100   = 0x03,  ///< 100 kHz
  CHANNEL_SPACING_250   = 0x04,  ///< 250 kHz
  CHANNEL_SPACING_800   = 0x05,  ///< 800 kHz
  CHANNEL_SPACING_1200  = 0x06,  ///< 1200 kHz
  CHANNEL_SPACING_UNDEF = 0x0F,
};

/// Modulation types as defined in the RCP API.
enum {
  MODULATION_OFDM  = 0,  ///< OFDM
  MODULATION_OQPSK = 1,  ///< OQPSK
  MODULATION_BPSK  = 2,  ///< BPSK
  MODULATION_GFSK  = 3,  ///< GFSK
  MODULATION_2FSK  = 4,  ///< 2FSK
  MODULATION_UNDEFINED,
};

/// OFDM PhyMode values as defined in PHY specification.
enum {
  OFDM_MCS_0 = 0,  ///< OFDM MCS 0
  OFDM_MCS_1 = 1,  ///< OFDM MCS 1
  OFDM_MCS_2 = 2,  ///< OFDM MCS 2
  OFDM_MCS_3 = 3,  ///< OFDM MCS 3
  OFDM_MCS_4 = 4,  ///< OFDM MCS 4
  OFDM_MCS_5 = 5,  ///< OFDM MCS 5
  OFDM_MCS_6 = 6,  ///< OFDM MCS 6
  OFDM_MCS_7 = 7,  ///< OFDM MCS 7. Currently non-standard modulation.
  OFDM_MCS_NUMBER,
  OFDM_MCS_UNDEFINED
};

/// PHY parameters.
typedef struct phy_params {
  uint8_t rail_phy_mode_id;     // RAIL PHY operating mode ID
  uint8_t phy_mode_id;          // PHY operating mode ID (0xFF if invalid)
  uint8_t modulation;           // Modulation
  uint32_t datarate;            // Datarate (bits/second)
  uint8_t op_mode;              // PHY operating mode (0 if invalid)
  uint8_t ofdm_mcs;             // OFDM PHY mode
  bool fec;                     // FEC enabled
} sl_wisun_phy_params_t;

/// Channel Plan parameters.
typedef struct chan_params {
  uint8_t reg_domain;         // Regulatory domain
  uint8_t op_class;           // Operating class (0 if not specified by FAN 1.0)
  uint8_t regional_reg;       // Regional regulation (unused for now)
  uint8_t chan_plan_id;       // Channel plan id (0xFF if not specified by FAN 1.1)
  uint32_t chan0_freq_hz;     // First channel center frequency (hertz)
  uint32_t chan_spacing_hz;   // Separation between adjacent channels (hertz)
  uint16_t chan_count;        // Total number of channels (before masking) (TotalNumChan)
  uint8_t phy_mode_ids[8];    // PHY operating mode IDs
  const char *chan_allowed;   // Regulatory channel mask, 1 if allowed (e.g. "0-3,13,42-255")
} sl_wisun_chan_params_t;

/**************************************************************************//**
 * Check if PHY and Channel Plan parameters are compatible.
 *
 * @param[in] phy_params Pointer to PHY parameters
 * @param[in] chan_params Pointer to channel parameters
 * @return true if the PHY is valid for the channel plan, false otherwise
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
bool ws_regdb_check_phy_chan_compat(const sl_wisun_phy_params_t *phy_params, const sl_wisun_chan_params_t *chan_params);

/**************************************************************************//**
 * Get the PHY parameters associated to an Operating Mode.
 *
 * @param[in] operating_mode Operating Mode
 * @return Pointer to the PHY parameters, or NULL if not found
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
const sl_wisun_phy_params_t *ws_regdb_phy_params_from_mode(uint8_t operating_mode);

/**************************************************************************//**
 * Get the PHY parameters associated to a PHY Mode ID.
 *
 * @param[in] operating_mode PHY Mode ID
 * @return Pointer to the PHY parameters, or NULL if not found
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
const sl_wisun_phy_params_t *ws_regdb_phy_params_from_id(uint8_t phy_mode_id);

/**************************************************************************//**
 * Retrieve the PHY parameters based on the PHY Mode ID or the Operating Mode.
 *
 * @param[in] operating_mode PHY Mode ID
 * @param[in] operating_mode Operating Mode
 * @return Pointer to the PHY parameters, or NULL if not found
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
const sl_wisun_phy_params_t *ws_regdb_phy_params(uint8_t phy_mode_id, uint8_t operating_mode);

/**************************************************************************//**
 * Retrieve the Channel Plan parameters based on Regulatory Domain, and the
 * ChanPlanID / Operating Class.
 *
 * @param[in] reg_domain Regulatory Domain
 * @param[in] chan_plan_id Channel Plan ID
 * @param[in] operating_class Operating Class
 * @return Pointer to the Channel Plan parameters, or NULL if not found
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
const sl_wisun_chan_params_t *ws_regdb_chan_params(uint8_t reg_domain, uint8_t chan_plan_id, uint8_t operating_class);

/**************************************************************************//**
 * Retrieve the ID as defined in Wi-SUN specification for the given channel spacing.
 *
 * @param[in] val Channel spacing in Hz
 * @return Channel spacing ID, or CHANNEL_SPACING_UNDEF if not found
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
uint8_t ws_regdb_chan_spacing_id(uint32_t val);

/**************************************************************************//**
 * Retrieve the channel spacing value in Hz for the given channel spacing ID.
 *
 * @param[in] id Channel Spacing ID
 * @return Channel Spacing in Hz, or 0 if not found
 *
 * Available in libraries: Full, FFN, LFN (see @ref API_AVAILABILITY)
 *****************************************************************************/
uint32_t ws_regdb_chan_spacing_value(uint8_t id);

#endif /* SL_WISUN_REGDB_H */
