/***************************************************************************//**
 * @file
 * @brief Clock Manager Oscillator Calibration Override API implementations.
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

#include "sl_clock_manager.h"
#include "sl_clock_manager_oscillator_calibration_override.h"
#include "nvm3.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Allocated range of NVM3 IDs for oscillator calibration usage */
#define HFXO_CTUNE_NVM3_RESERVED_ID  (0x89800UL)

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Configure Oscillator CTune settings using key values stored in NVM3.
 ******************************************************************************/
sl_status_t sl_clock_manager_init_oscillator_calibration_override(void)
{
  sl_status_t status = SL_STATUS_OK;
  uint32_t hfxo_calibration;

  status = sl_clock_manager_read_hfxo_calibration_override(&hfxo_calibration);

  if (status != SL_STATUS_OK ) {
    return status;
  }

  status = slx_clock_manager_hfxo_calibrate_ctune(hfxo_calibration);

  return status;
}

/***************************************************************************//**
 * Write HFXO calibration value to NVM3.
 ******************************************************************************/
sl_status_t sl_clock_manager_write_hfxo_calibration_override(uint32_t ctune)
{
  // Ensure CTUNE value is within the allowable range.
  if (ctune > (_HFXO_XTALCTRL_CTUNEXIANA_MASK >> _HFXO_XTALCTRL_CTUNEXIANA_SHIFT)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status;
  nvm3_ObjectKey_t object_id = HFXO_CTUNE_NVM3_RESERVED_ID;

  // Write NVM3 HFXO calibration value.
  status = nvm3_writeData(nvm3_defaultHandle, object_id, &ctune, sizeof(ctune));

  return status;
}

/***************************************************************************//**
 * Read HFXO calibration value from NVM3.
 ******************************************************************************/
sl_status_t sl_clock_manager_read_hfxo_calibration_override(uint32_t *ctune)
{
  // Ensure CTUNE is not NULL.
  if (ctune == NULL ) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_status_t status;
  nvm3_ObjectKey_t object_id = HFXO_CTUNE_NVM3_RESERVED_ID;

  // Read NVM3 HFXO calibration value.
  status = nvm3_readData(nvm3_defaultHandle, object_id, ctune, sizeof(ctune));

  if (status == SL_STATUS_NOT_FOUND) {
    *ctune = 0xFFFF;
  }

  return status;
}

/***************************************************************************//**
 * Delete HFXO calibration value from NVM3.
 ******************************************************************************/
sl_status_t sl_clock_manager_delete_hfxo_calibration_override(void)
{
  sl_status_t status;
  nvm3_ObjectKey_t object_id = HFXO_CTUNE_NVM3_RESERVED_ID;

  status = nvm3_deleteObject(nvm3_defaultHandle, object_id);

  return status;
}
