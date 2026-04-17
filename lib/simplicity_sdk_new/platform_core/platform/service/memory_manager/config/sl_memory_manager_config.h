/***************************************************************************//**
 * @file
 * @brief Memory Heap Allocator configuration file.
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_MEMORY_MANAGER_CONFIG_H
#define SL_MEMORY_MANAGER_CONFIG_H

// <h> Memory Manager Configuration

// <o SL_MEMORY_MANAGER_BLOCK_ALLOCATION_MIN_SIZE> Minimum block allocation size
// <32-128:8>
// <i> Minimum block allocation size to avoid creating a block too small while splitting up an allocated block.
// <i> Size expressed in bytes and can only be a multiple of 8 bytes for the proper data alignment management done by the dynamic allocator malloc() function.
// <i> Default: 32
#define SL_MEMORY_MANAGER_BLOCK_ALLOCATION_MIN_SIZE   (32)

// <q SL_MEMORY_MANAGER_STATISTICS_API_ENABLE> Enables the statistics API.
// <i> Setting this configuration to 0 will make all the statistics API return 0.
// <i> Default: 1
#define SL_MEMORY_MANAGER_STATISTICS_API_ENABLE  1

// <q SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE> Enables the double free protection for the pool.
// <i> Setting this configuration to 0 will disable the double free protection for the memory pool. saving ~100 bytes of code.
// <i> This setting has no effect on the memory pools power aware version.
// <i> Default: 1
#define SL_MEMORY_MANAGER_POOL_DOUBLE_FREE_PROTECTION_ENABLE 1

// </h>

// <<< end of configuration section >>>

// Keep this configuration outside the configuration section until the feature is ready for release.
// <q SL_MEMORY_MANAGER_HEAP_FALLBACK_EN> Enables the heap fallback mechanism.
// <i> Setting this configuration to 0 will disable the fallback mechanism on DTCM and PSRAM.
// <i> If this configuration is disabled, the memory manager will not attempt to use alternative memory regions for allocations even if using the fallback parameters.
// <i> Default: 1
#define SL_MEMORY_MANAGER_HEAP_FALLBACK_EN 1

#endif /* SL_MEMORY_MANAGER_CONFIG_H */
