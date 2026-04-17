/***************************************************************************//**
 * @file
 * @brief Clock Manager Oscillator Calibration Override APIs.
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

#ifndef SL_CLOCK_MANAGER_OSCILLATOR_CALIBRATION_OVERRIDE_H
#define SL_CLOCK_MANAGER_OSCILLATOR_CALIBRATION_OVERRIDE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup clock_manager Clock Manager
 * @{
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************//**
 * Configure Oscillator CTune settings using override key values stored in
 * NVM3.
 *
 * @return  Status code.
 *          SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sl_clock_manager_init_oscillator_calibration_override(void);

/***************************************************************************//**
 * Write HFXO override calibration value to NVM3.
 *
 * @param[in] ctune     The HFXO override CTune value to write to NVM3.
 *
 * @return  Status code.
 *          SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sl_clock_manager_write_hfxo_calibration_override(uint32_t ctune);

/***************************************************************************//**
 * Read HFXO override calibration value from NVM3.
 *
 * @param[out] ctune    HFXO override CTune value read from NVM3.
 *
 * @return  Status code.
 *          SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sl_clock_manager_read_hfxo_calibration_override(uint32_t *ctune);

/***************************************************************************//**
 * Delete HFXO override calibration value from NVM3.
 *
 * @return  Status code.
 *          SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sl_clock_manager_delete_hfxo_calibration_override(void);

/** @} (end addtogroup clock_manager) */

#ifdef __cplusplus
}
#endif

#endif // SL_CLOCK_MANAGER_OSCILLATOR_CALIBRATION_OVERRIDE_H
