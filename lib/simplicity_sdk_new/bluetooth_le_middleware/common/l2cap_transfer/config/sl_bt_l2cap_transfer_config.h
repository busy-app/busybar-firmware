/***************************************************************************//**
 * @file
 * @brief L2CAP transfer configuration file
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

#ifndef SL_BT_L2CAP_TRANSFER_CONFIG_H
#define SL_BT_L2CAP_TRANSFER_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup l2cap_transfer
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <e SL_BT_L2CAP_TRANSFER_CONFIG_ACCEPT_PRIOR_CHANNELS> Enable prior channels
// <i> Default: 1
#define SL_BT_L2CAP_TRANSFER_CONFIG_ACCEPT_PRIOR_CHANNELS   1

// <o SL_BT_L2CAP_TRANSFER_CONFIG_PRIOR_CHANNEL_COUNT> Prior channels count <1-32>
// <i> Set the prior channel count (if prior channels are enabled).
// <i> Matching SL_BT_CONFIG_MAX_CONNECTIONS on the NCP target device can
// <i> help achieve maximum performance if every connection uses L2CAP, but may
// <i> not be necessary depending on the host application's connection strategy.
// <i> Note: larger values increase static memory usage.
// <i> Default: 8
#define SL_BT_L2CAP_TRANSFER_CONFIG_PRIOR_CHANNEL_COUNT     8

// </e>

// <<< end of configuration section >>>

/** @} (end addtogroup l2cap_transfer) */
#endif // SL_BT_L2CAP_TRANSFER_CONFIG_H
