/***************************************************************************//**
 * @file
 * @brief Assigns the fixed RTT_BUFFER_INDEXES to components who uses RTT
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
 
// This header assigns the fixed RTT_BUFFER_INDEXES to components who uses RTT.

#ifndef SL_RTT_BUFFER_INDEX_H
#define SL_RTT_BUFFER_INDEX_H

// The default Buffer index of RTT buffer used for reading from the target & writing to the target is 0.
#define SL_IOSTREAM_RTT_BUFFER_INDEX               0

// The default RTT_BUFFER_INDEX for SEGGER_SYSVIEW_RTT_CHANNEL is 1.
#define SL_SYSTEMVIEW_RTT_BUFFER_INDEX             1

#define SL_MEMORY_PROFILER_RTT_BUFFER_INDEX        2

#define SL_BTCTRL_TRACE_RTT_BUFFER_INDEX           3

#define SL_BGAPI_TRACE_RTT_BUFFER_INDEX            4

#define SL_OPENTHREAD_LOGGING_RTT_BUFFER_INDEX     5

#define SL_OPENTHREAD_UART_RTT_BUFFER_INDEX        6

#define SL_BTMESH_DEBUG_RTT_BUFFER_INDEX           7

#define SL_IOSTREAM_VUART_RTT_BUFFER_INDEX         8

#define SL_BTCTRL_SEQUENCER_TRACE_RTT_BUFFER_INDEX 9

#endif // SL_RTT_BUFFER_INDEX_H
