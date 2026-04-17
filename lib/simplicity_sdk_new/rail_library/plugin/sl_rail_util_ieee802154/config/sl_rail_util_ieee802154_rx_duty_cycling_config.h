/***************************************************************************//**
 * @file
 * @brief IEEE802.15.4 Duty Cycling PHY configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_RAIL_UITL_IEEE802154_RX_DUTY_CYCLING_CONFIG_H
#define SL_RAIL_UITL_IEEE802154_RX_DUTY_CYCLING_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h> IEEE802.15.4 Duty Cycling PHY Configuration
// <q SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_DEFAULT_ENABLED> Default enabled
// <i> (Enable)RX Dutycycling is enabled by default.
// <i> (Disable)RX Dutycycling is disabled by default.
// <i> Default: 0
#define SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_DEFAULT_ENABLED 0
// <q SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_RUNTIME_PHY_SELECT> Runtime select
// <i> (Enable)RX Dutycycling feature can be modified at runtime.
// <i> (Disable)RX Dutycycling feature can not be modified at runtime.
// <i> Default: 1
#define SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_RUNTIME_PHY_SELECT 1
// </h>
// <<< end of configuration section >>>

#endif //SL_RAIL_UITL_IEEE802154_RX_DUTY_CYCLING_CONFIG_H
