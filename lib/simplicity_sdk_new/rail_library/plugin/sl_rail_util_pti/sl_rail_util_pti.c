/***************************************************************************//**
 * @file
 * @brief Packet Trace Information
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

#include "em_device.h"
#include "sl_gpio.h"

#include "sl_rail.h"
#include "rail_types.h" // Support RAIL 2.x PTI modes
#include "sl_rail_util_pti.h"
#include "sl_rail_util_pti_config.h"

void sl_rail_util_pti_init(void)
{
  sl_rail_pti_config_t rail_pti_config = {
    .mode = SL_RAIL_UTIL_PTI_MODE,
    .baud = SL_RAIL_UTIL_PTI_BAUD_RATE_HZ,
#if defined(SL_RAIL_UTIL_PTI_DOUT_PORT) && defined(SL_RAIL_UTIL_PTI_DOUT_PIN)
    .dout_port = (uint8_t)SL_RAIL_UTIL_PTI_DOUT_PORT,
    .dout_pin = SL_RAIL_UTIL_PTI_DOUT_PIN,
#endif // dout support
#if defined(SL_RAIL_UTIL_PTI_DCLK_PORT) && defined(SL_RAIL_UTIL_PTI_DCLK_PIN)
    .dclk_port = (uint8_t)SL_RAIL_UTIL_PTI_DCLK_PORT,
    .dclk_pin = SL_RAIL_UTIL_PTI_DCLK_PIN,
#endif // dclk support
#if defined(SL_RAIL_UTIL_PTI_DFRAME_PORT) && defined(SL_RAIL_UTIL_PTI_DFRAME_PIN)
    .dframe_port = (uint8_t)SL_RAIL_UTIL_PTI_DFRAME_PORT,
    .dframe_pin = SL_RAIL_UTIL_PTI_DFRAME_PIN,
#endif // dframe support
  };

  (void) sl_rail_config_pti(SL_RAIL_EFR32_HANDLE, &rail_pti_config);
}
