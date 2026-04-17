/***************************************************************************//**
 * @file sl_iperf_config.h
 * @brief iPerf component configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_IPERF_CONFIG_H
#define SL_IPERF_CONFIG_H

/**************************************************************************//**
 * @defgroup SL_IPERF_CONFIG Configurations
 * @ingroup SL_IPERF
 * @{
 *****************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h>iPerf common configuration
// <o SL_IPERF_SOCKET_ADDR_LEN> Set the address length buffer size
// <i> This parameter must be set for different port of network layer
#define SL_IPERF_SOCKET_ADDR_LEN                   32U ///< Set the address length buffer size

// <o SL_IPERF_MESSAGE_QUEUE_SIZE> Set test message queue size
// <i> Default value: 16
#define SL_IPERF_MESSAGE_QUEUE_SIZE                16U ///< Set test message queue size

// <o SL_IPERF_STACK_SIZE_WORD> Set test thread stack size in word depth
// <i> Default value: 256
#define SL_IPERF_STACK_SIZE_WORD                   256U ///< Set test thread stack size in word depth

// <o SL_IPERF_BUFFER_SIZE> Server receiver buffer size
// <i> Default value 1234 (UDPv6 size without fragmentation)
#define SL_IPERF_BUFFER_SIZE                       1232U ///< Server receiver buffer size

// <o SL_IPERF_LOG_JSON_INDENT> Set json formatted log indent in space count
// <i> Default value: 2
#define SL_IPERF_LOG_JSON_INDENT                   2U ///< Set json formatted log indent in space count

// <q SL_IPERF_VERBOSE_MODE> Enable verbose prints
// <i> Default value: 0
#define SL_IPERF_VERBOSE_MODE                      0U ///< Enable verbose prints

// <o SL_IPERF_DEFAULT_INTERVAL_MS> Default statistic update interval
// <i> Default value 1000
#define SL_IPERF_DEFAULT_INTERVAL_MS               1000U ///< Default statistic update interval

// <o SL_IPERF_MAX_TEST_TIMEOUT_MS> Maximum possible additional test timeout after duration time
// <i> Default value 2000
#define SL_IPERF_MAX_TEST_TIMEOUT_MS               2000U ///< Maximum possible additional test timeout after duration time
// </h>

// <h>iPerf Server configuration
// <o SL_IPERF_SERVER_DEFAULT_PORT> Default Server port
// <i> Default value: 5001
#define SL_IPERF_SERVER_DEFAULT_PORT               5001U ///< Default Server port
// </h>

// <h>iPerf Client configuration
// <o SL_IPERF_CLIENT_DEFAULT_SEND_TIME_MS> Default sending time in millisec
// <i> Default value: 10000
#define SL_IPERF_CLIENT_DEFAULT_SEND_TIME_MS       10000U ///< Default sending time in millisec

// <o SL_IPERF_CLIENT_DEFAULT_BANDWIDTH_BPS> Default bandwidth in bits/sec
// <i> Default value: 20000
#define SL_IPERF_CLIENT_DEFAULT_BANDWIDTH_BPS      20000U ///< Default bandwidth in bits/sec
// </h>

// <<< end of configuration section >>>

/** @}*/

#endif // SL_IPERF_CONFIG_H
