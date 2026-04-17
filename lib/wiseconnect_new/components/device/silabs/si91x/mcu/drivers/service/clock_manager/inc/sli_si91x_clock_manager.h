/******************************************************************************
 * @file sli_si91x_clock_manager.h
 * @brief Clock Manager Internal API implementation
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

#ifndef SLI_SI91X_CLOCK_MANAGER_H
#define SLI_SI91X_CLOCK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_clock_manager.h"
#include "sl_si91x_power_manager.h"

/************************************************************************************
 * @addtogroup CLOCK-MANAGER Clock Manager
 * @ingroup SI91X_SERVICE_APIS
 * @{
 ************************************************************************************/
// -----------------------------------------------------------------------------------
// GLOBAL DEFINES / MACROS
// -----------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------
// DATA TYPES
// -----------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------
// GLOBAL FUNCTION PROTOTYPES
// -----------------------------------------------------------------------------------
/***************************************************************************/
/**
 * @brief Configures the clocks based on the requested power state and power mode.
 * 
 * @param[in] power_state Enum values of different power states. See \ref sl_power_state_t for more info.
 * @param[in] power_mode  Clock scaling mode in PS4 and PS3 power states.
 *                        Possible values:
 *                        - SL_SI91X_POWER_MANAGER_POWERSAVE   (Minimum supported frequency in a power state)
 *                        - SL_SI91X_POWER_MANAGER_PERFORMANCE (Maximum supported frequency in a power state)
 * 
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK  - Success.
 *         - Corresponding error code on failure.
 * 
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 **************************************************************************************************/
sl_status_t sli_si91x_clock_manager_config_clks_on_ps_change(sl_power_state_t power_state, boolean_t power_mode);
/*******************************************************************************
 * @brief This API Switch Subsystems' Ref clocks to MHz RC,Set M4 SOC and QSPI/QSPI2 clock to Ref clock
 ******************************************************************************/
sl_status_t sli_si91x_config_clocks_to_mhz_rc(void);
/// @} end group CLOCK-MANAGER ******************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SLI_SI91X_CLOCK_MANAGER_H */
