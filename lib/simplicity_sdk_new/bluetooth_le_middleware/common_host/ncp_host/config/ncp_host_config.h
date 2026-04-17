/***************************************************************************//**
 * @file
 * @brief NCP host application module config.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef NCP_HOST_CONFIG_H
#define NCP_HOST_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Peek Configuration

// <q PEEK_US_SLEEP> 1
// <i> Microsec delay used between peek function calls.
#ifndef PEEK_US_SLEEP
#define PEEK_US_SLEEP            1
#endif

// <q MSG_RECV_TIMEOUT_MS> 1
// <i> Millisec timeout for peek function calls.
#ifndef MSG_RECV_TIMEOUT_MS
#define MSG_RECV_TIMEOUT_MS      10
#endif

// </h> End Peek Configuration

// <h> Reboot Configuration

// During init, system reboot command is not sent because it can break e.g. the
// multi-protocol use case when used with CPC. Therefore, we just wait for the
// boot event without sending the first system reboot command.
#ifndef NCP_REBOOT_TIMEOUT_INIT_MS
#define NCP_REBOOT_TIMEOUT_INIT_MS 1000
#endif

// If no boot event is received after the initial timeout, a system reboot
// command is sent to the target. After the system reboot, we should let enough
// time for the target to boot up and for the CPC to initialize.
#ifndef NCP_REBOOT_TIMEOUT_RETRY_MS
#define NCP_REBOOT_TIMEOUT_RETRY_MS 10000
#endif

// System reboot is repeated NCP_REBOOT_RETRY_COUNT times.
#ifndef NCP_REBOOT_RETRY_COUNT
#define NCP_REBOOT_RETRY_COUNT 3
#endif

// Wait for system boot event after init.
#ifndef WAIT_FOR_SYSTEM_BOOT
#define WAIT_FOR_SYSTEM_BOOT          1
#endif

// Accept DFU boot event after init - enable it for the UART DFU host application
#ifndef ACCEPT_DFU_BOOT
#define ACCEPT_DFU_BOOT               0
#endif

// </h> End Reboot Configuration

// <<< end of configuration section >>>

#endif // NCP_HOST_CONFIG_H
