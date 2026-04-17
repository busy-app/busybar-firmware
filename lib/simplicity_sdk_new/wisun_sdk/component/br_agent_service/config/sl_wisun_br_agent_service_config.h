/***************************************************************************//**
 * @file sl_wisun_br_agent_service_config.h
 * @brief Wi-SUN Border Router Agent Service configuration
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
#ifndef SL_WISUN_BR_AGENT_SERVICE_CONFIG_H
#define SL_WISUN_BR_AGENT_SERVICE_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Wi-SUN Border Router Agent Service configuration

// <o SL_WISUN_BR_AGENT_SERVICE_SERVER_PORT> Default listening port of the local agent service server
// <i> Default: 11501
// <1-65536>
#define SL_WISUN_BR_AGENT_SERVICE_SERVER_PORT               11501U ///< Default listening port of the local agent service server

// <o SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_PORT> Default port of the Border Router Bridge Agent to send messages
// <i> Default: 11500
// <1-65536>
#define SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_PORT               11500U ///< Default port of the remote host to send messages

// <s SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_ADDR> Default address of the Border Router Bridge Agent to send messages
// <i> This address is used by default as the destination address of the remote host to send messages
// <i> Default: 2001:db8::1
#define SL_WISUN_BR_BRIDGE_AGENT_DEFAULT_ADDR                "2001:db8::1" ///< Default address of the remote host to send messages

// </h>

// <<< end of configuration section >>>

#endif // SL_WISUN_BR_AGENT_SERVICE_CONFIG_H
