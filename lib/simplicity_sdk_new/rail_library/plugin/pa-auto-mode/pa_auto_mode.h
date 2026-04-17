/***************************************************************************//**
 * @file
 * @brief This file contains the type definitions for RAIL structures, enums,
 *   and other types.
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

#ifndef __PA_AUTO_MODE_H
#define __PA_AUTO_MODE_H

#include "rail.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup PA Power Amplifier (PA)
 * @ingroup Transmit
 * @{
 */

/**
 * @enum RAIL_PaBand_t
 * @brief Enum used to specify the band for a PA
 *
 * @deprecated This RAIL 2.x enum has been eliminated in RAIL 3 where
 *   PA selection is incorporated into the PA tables themselves.
 */
RAIL_ENUM(RAIL_PaBand_t) {
  /**
   * Indicates a 2.4 GHz band PA.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_PA_BAND_2P4GIG,
  /**
   * Indicates a Sub-GHz band PA.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_PA_BAND_SUBGIG,
  /**
   * A count of the choices in this enumeration. Must be last.
   *
   * @deprecated This RAIL 2.x enum value has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_PA_BAND_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using RAIL_ENUM
#define RAIL_PA_BAND_2P4GIG ((RAIL_PaBand_t)RAIL_PA_BAND_2P4GIG)
#define RAIL_PA_BAND_SUBGIG ((RAIL_PaBand_t)RAIL_PA_BAND_SUBGIG)
#define RAIL_PA_BAND_COUNT  ((RAIL_PaBand_t)RAIL_PA_BAND_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct RAIL_PaAutoModeConfigEntry_t
 * @brief Struct to ease specification of appropriate ranges
 *   within which a PA should be used.
 *
 * @deprecated This RAIL 2.x type has been eliminated in RAIL 3 where
 *   PA selection is incorporated into the PA tables themselves.
 */
typedef struct RAIL_PaAutoModeConfigEntry {
  /**
   * The minimum (inclusive) deci-dBm power to use with this entry.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_TxPower_t min;
  /**
   * The maximum (inclusive) deci-dBm power to use with this entry.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_TxPower_t max;
  /**
   * The PA that this range of powers applies to.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_TxPowerMode_t mode;
  /**
   * The RF band that this PA works with.
   *
   * @deprecated This RAIL 2.x field has been eliminated in RAIL 3 where
   *   PA selection is incorporated into the PA tables themselves.
   */
  RAIL_PaBand_t band;
} RAIL_PaAutoModeConfigEntry_t;

/**
 * The actual PA auto mode configuration structure used by the auto mode plugin
 * to control output power.
 *
 * @deprecated This RAIL 2.x variable has been eliminated in RAIL 3 where
 *   PA selection is incorporated into the PA tables themselves.
 */
extern const RAIL_PaAutoModeConfigEntry_t *RAIL_PaAutoModeConfig;

/**
 * Configure the PA auto mode entries.
 *
 * @param[in] railHandle A RAIL instance handle.
 * @param[in] paAutoModeEntry A pointer to entries used to configure PA auto mode decision points.
 *   The final entry must set its \ref RAIL_PaAutoModeConfigEntry_t::band to \ref RAIL_PA_BAND_COUNT.
 * @return Status parameter indicating success of function call.
 *
 * @deprecated This RAIL 2.x function has been eliminated in RAIL 3 where
 *   PA selection is incorporated into the PA tables themselves.
 */
RAIL_Status_t RAIL_ConfigPaAutoEntry(RAIL_Handle_t railHandle,
                                     const RAIL_PaAutoModeConfigEntry_t *paAutoModeEntry);

/** @} */ // PA Power Amplifier (PA)

#ifdef RAIL_PRIVATE_BUILD
#include "pa_auto_mode_internal.h"
#endif

#ifdef __cplusplus
}
#endif

#endif // __PA_AUTO_MODE_H
