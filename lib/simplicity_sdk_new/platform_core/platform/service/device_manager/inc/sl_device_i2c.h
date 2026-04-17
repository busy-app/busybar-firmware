/***************************************************************************//**
 * @file
 * @brief Device Manager I2C.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/

#ifndef SL_DEVICE_I2C_H
#define SL_DEVICE_I2C_H

#include "sl_enum.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup device_i2c Device Manager I2C
 * @details
 * ## Overview
 *
 * The Device Manager I2C component defines the macros,
 * structures, and enums that are used commonly across I2C driver and
 * peripheral.
 *
 * @{
 ******************************************************************************/

// ----------------------------------------------------------------------------
// ENUMS

/// Leader or Follower mode selection enum.
SL_ENUM(sl_i2c_operating_mode_t) {
  SL_I2C_LEADER_MODE = 0,   ///< I2C Leader Mode.
  SL_I2C_FOLLOWER_MODE,     ///< I2C Follower Mode.
};

/// FIFO Threshold enum.
#if defined(SL_CATALOG_I2C_FIFO_PRESENT)
SL_ENUM(sl_i2c_fifo_threshold_t) {
  SL_I2C_FIFO_THRESHOLD_1 = 0,    ///< I2C FIFO threshold value 1.Interrupt will be triggered for single byte.
  SL_I2C_FIFO_THRESHOLD_2 = 1,    ///< I2C FIFO threshold value 2.Interrupt will be triggered for two bytes.
};
#endif

/// Clock high to low ratio settings.
SL_ENUM(sl_i2c_clock_hlr_t) {
  SL_I2C_CLK_HLR_STANDARD     = 0,    ///< Ratio is 4:4.
  SL_I2C_CLK_HLR_ASYMMETRIC,          ///< Ratio is 6:3.
  SL_I2C_CLK_HLR_FAST,                ///< Ratio is 11:3.
};

// ----------------------------------------------------------------------------
// DEFINES

// Masks for 7-bit and 10-bit follower addresses.
#define SL_I2C_7BIT_FOLLOWER_ADDRESS_MASK   (0xFE)
#define SL_I2C_FIRST_BYTE_10BIT_ADDR_MASK   (0xF0)

/// Validation of operating mode
#define SL_I2C_OPERATING_MODE_IS_VALID(operating_mode)    ((operating_mode == SL_I2C_LEADER_MODE) || (operating_mode == SL_I2C_FOLLOWER_MODE))

// Validation macro for I2C freq mode.
#define SL_I2C_FREQ_MODE_IS_VALID(freq_mode)    ((freq_mode == SL_I2C_FREQ_STANDARD_MODE) \
                                                 || (freq_mode == SL_I2C_FREQ_FAST_MODE)  \
                                                 || (freq_mode == SL_I2C_FREQ_FASTPLUS_MODE))


/** @} (end addtogroup device_i2c) */

#ifdef __cplusplus
}
#endif

#endif // SL_DEVICE_I2C_H
