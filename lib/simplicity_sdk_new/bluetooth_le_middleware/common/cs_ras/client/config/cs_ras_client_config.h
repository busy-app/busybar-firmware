/***************************************************************************//**
 * @file
 * @brief CS RAS Client configuration
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

#ifndef CS_RAS_CLIENT_CONFIG_H
#define CS_RAS_CLIENT_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> General

// <q CS_RAS_CLIENT_CONFIG_LOG_ENABLE> Log
// <i> Default: 0
#ifndef CS_RAS_CLIENT_CONFIG_LOG_ENABLE
#define CS_RAS_CLIENT_CONFIG_LOG_ENABLE                                              0
#endif

// </h>

// <h> Timeout

// <o CS_RAS_CLIENT_REAL_TIME_RANGING_DATA_PROCEDURE_TIMEOUT_MS> Real-Time Ranging Data timeout from procedure start [ms] <1..5000>
// <i> Timeout for data reception after procedure is started.
// <i> Default: 5000
#ifndef CS_RAS_CLIENT_REAL_TIME_RANGING_DATA_PROCEDURE_TIMEOUT_MS
#define CS_RAS_CLIENT_REAL_TIME_RANGING_DATA_PROCEDURE_TIMEOUT_MS                    5000
#endif

// <o CS_RAS_CLIENT_RANGING_DATA_READY_PROCEDURE_TIMEOUT_MS> Ranging Data Ready Timeout from procedure start [ms] <1..5000>
// <i> Timeout for data Ranging Data Ready indication/notification after procedure started.
// <i> Default: 5000
#ifndef CS_RAS_CLIENT_RANGING_DATA_READY_PROCEDURE_TIMEOUT_MS
#define CS_RAS_CLIENT_RANGING_DATA_READY_PROCEDURE_TIMEOUT_MS                        5000
#endif

// <o CS_RAS_CLIENT_CONTROL_POINT_TIMEOUT_MS> Control Point response timeout [ms] <1..5000>
// <i> Timeout for RAS Control Point response after a request is sent.
// <i> Default: 5000
#ifndef CS_RAS_CLIENT_CONTROL_POINT_TIMEOUT_MS
#define CS_RAS_CLIENT_CONTROL_POINT_TIMEOUT_MS                                       5000
#endif

// </h>

// <<< end of configuration section >>>

#endif // CS_RAS_CLIENT_CONFIG_H
