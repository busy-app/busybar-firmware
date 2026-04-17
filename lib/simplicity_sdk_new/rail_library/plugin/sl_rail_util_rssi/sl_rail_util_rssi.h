/***************************************************************************//**
 * @file
 * @brief Default RSSI setup
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_RAIL_UTIL_RSSI_H
#define SL_RAIL_UTIL_RSSI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sl_status.h"

// | Region | SubRegion            | NVM3 Key<19:16> | NVM3 Key<15:0>
// | Common | Antenna Calibration  | 0x8             | 0x9800 - 0x98ff
// where Platform owns 0x89800-0x8987f and RAIL owns 0x89880-0x898ff.
#define SL_RAIL_UTIL_RSSI_NVM_DATA_TAG (0x89880UL)

/**
 * Initialize the default RSSI.
 *
 * @note: This function should be called during application initialization
 *        before \ref RAIL_Init has been called.
 */
void sl_rail_util_rssi_init(void);

/**
 * Sets the user specified RSSI offset(in dB) into NVM3.
 * @param rssiOffsetDb RSSI offset value(in dB) to be stored in NVM3.
 * @return RAIL status indicating success of the function call.
 *
 * @note: This function should be called during application initialization
 *        after NVM3 is initialized. This offset will be applied from next
 *        boot.
 */
sl_status_t sl_rail_util_write_nvm_rssi(int8_t rssiOffsetDb);

/**
 * Gets RSSI offset(in dB) from NVM3.
 * If there is no RSSI offset programmed into NVM3 then it provides the default
 * value which is chip specific.
 * @return RSSI offset(in dB) read from NVM3.
 *
 * @note: This function should be called during application initialization
 *        after NVM3 is initialized and before \ref sl_rail_util_rssi_init
 *        has been called.
 */
int8_t sl_rail_util_read_nvm_rssi(void);

#ifdef __cplusplus
}
#endif

#endif // SL_RAIL_UTIL_RSSI_H
