/***************************************************************************//**
 * @file sl_wisun_cli_config.h
 * @brief Wi-SUN CLI component configuration
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
#ifndef SL_WISUN_CLI_CONFIG_H
#define SL_WISUN_CLI_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Wi-SUN CLI configuration

// <q SL_WISUN_CLI_ENABLED> Enable All CLI functions
// <i> If enabled, CLI functions are available
#define SL_WISUN_CLI_ENABLED                      1

// <q SL_WISUN_PING_CLI_ENABLED> Enable Ping component specific CLI functions
// <i> If enabled, Ping CLI functions are available
#define SL_WISUN_PING_CLI_ENABLED                 1

// <q SL_WISUN_OTA_DFU_CLI_ENABLED> Enable OTA DFU component specific CLI functions
// <i> If enabled, OTA DFU CLI functions are available
#define SL_WISUN_OTA_DFU_CLI_ENABLED              1

// <q SL_WISUN_NETWORK_MEASUREMENT_CLI_ENABLED> Enable Network Measurement component specific CLI functions
// <i> If enabled, Network Measurement CLI functions are available
#define SL_WISUN_NETWORK_MEASUREMENT_CLI_ENABLED  1

// <q SL_WISUN_IPERF_CLI_ENABLED> Enable iPerf component specific CLI functions
// <i> If enabled, iPerf CLI functions are available
#define SL_WISUN_IPERF_CLI_ENABLED                1

// <q SL_WISUN_COLLECTOR_CLI_ENABLED> Enable Collector component specific CLI functions
// <i> If enabled, Collector CLI functions are available
#define SL_WISUN_COLLECTOR_CLI_ENABLED            1

// <q SL_WISUN_CLIENT_CLI_ENABLED> Enable Client component specific CLI functions
// <i> If enabled, Client CLI functions are available
#define SL_WISUN_CLIENT_CLI_ENABLED               1

// <q SL_WISUN_CLI_UTIL_CLI_ENABLED> Enable CLI Util component specific CLI functions
// <i> If enabled, CLI Util CLI functions are available
#define SL_WISUN_CLI_UTIL_CLI_ENABLED             1

// <q SL_WISUN_APP_STATUS_CLI_ENABLED> Enable App Status component specific CLI functions
// <i> If enabled, App Status CLI functions are available
#define SL_WISUN_APP_STATUS_CLI_ENABLED           1

// <q SL_WISUN_RTT_LOGGER_CLI_ENABLED> Enable Wi-SUN RTT Logger component specific CLI functions
// <i> If enabled, RTT Logger CLI functions are available
#define SL_WISUN_RTT_LOGGER_CLI_ENABLED           1

// <q SL_WISUN_BR_AGENT_SERVICE_CLI_ENABLED> Enable Border Router Agent Service component specific CLI functions
// <i> If enabled, Border Router Agent Service CLI functions are available
#define SL_WISUN_BR_AGENT_SERVICE_CLI_ENABLED     1

// <q SL_WISUN_TRACE_LEVEL_CONTROL_CLI_ENABLED> Enable Wi-SUN trace level control specific CLI functions
// <i> If enabled, trace level control CLI functions are available
#define SL_WISUN_TRACE_LEVEL_CONTROL_CLI_ENABLED  0

// </h>

// <<< end of configuration section >>>

#endif // SL_WISUN_CLI_CONFIG_H
