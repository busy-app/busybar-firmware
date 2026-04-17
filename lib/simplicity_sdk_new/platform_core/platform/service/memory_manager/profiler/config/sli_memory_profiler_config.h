/***************************************************************************//**
 * @file
 * @brief Memory profiler configuration
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

#ifndef SLI_MEMORY_PROFILER_CONFIG_H
#define SLI_MEMORY_PROFILER_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Memory Profiler Settings

// <q SLI_MEMORY_PROFILER_ENABLE_OWNERSHIP_TRACKING> Enable tracking of memory allocation ownership
// <i> Default: 1
// <i>
// <i> Configures whether memory ownership tracking calls are built into the source
// <i> that uses the Memory Profiler API. If disabled, the memory ownership tracking
// <i> macros "SLI_MEMORY_PROFILER_TRACK_OWNERSHIP" expand to nothing to avoid the code
// <i> size overhead. Allocation tracking macros that take the ownership info
// <i> (such as "SLI_MEMORY_PROFILER_TRACK_ALLOC_WITH_OWNERSHIP") expand to normal allocation
// <i> tracking without the ownership information.
// <i>
// <i> Values:
// <i> * 0: Exclude memory ownership tracking from the code.
// <i> * 1: Include the memory ownership tracking to the code.
#define SLI_MEMORY_PROFILER_ENABLE_OWNERSHIP_TRACKING 1

// <o SLI_MEMORY_PROFILER_RTT_BUFFER_SIZE> RTT buffer size for event output <36-1024>
// <i> Default: 1024
// <i> This configuration specifies the size of the RTT buffer that is used for
// <i> outputting the Memory Profiler events.
#define SLI_MEMORY_PROFILER_RTT_BUFFER_SIZE  1024

// </h>
// <<< end of configuration section >>>

#endif /* SLI_MEMORY_PROFILER_CONFIG_H */
