/******************************************************************************
* @file sl_si91x_wireless_shutdown.c
* @brief  wireless shutdown
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
#include "sl_si91x_wireless_shutdown.h"
#include "rsi_power_save.h"
#include "rsi_rom_clks.h"

// -----------------------------------------------------------------------------
// Prototypes
/**
 * @fn         sl_si91x_wireless_shutdown
 * @brief      Configure power management settings for SI91X device
 * @param[in]  none
 * @return    none.
 * @section description
 * This API disables the ULP reference clocks, changes the TASS
 * Reference, and shuts down the power supply.
 */
void sl_si91x_wireless_shutdown(void)
{
  // This API is used to change the TASS reference clock to MHz
  RSI_ChangeTassRefClock();
  // This API is used to shutdown the power supply to wireless functionalities
  ps_wireless_shutdown();
}