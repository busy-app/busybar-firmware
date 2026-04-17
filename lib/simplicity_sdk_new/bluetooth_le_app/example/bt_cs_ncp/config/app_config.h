/***************************************************************************//**
 * @file
 * @brief CS Initiator NCP example configuration
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q ALWAYS_INIT_TRACE> Enable trace
// <i> If enabled, this option bypasses trace initialization with the ACP command CS_ACP_CMD_ENABLE_TRACE.
// <i> Default: 0
#define ALWAYS_INIT_TRACE                     0

// <q TRACE_RTL_LOGGING> Enable trace
// <i> If enabled, RTL logging will be visible in the trace output.
// <i> Default: 0
#define TRACE_RTL_LOGGING                     0

// <<< end of configuration section >>>

#endif // APP_CONFIG_H
