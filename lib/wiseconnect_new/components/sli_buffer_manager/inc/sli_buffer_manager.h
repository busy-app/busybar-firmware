/***************************************************************************/ /**
 * @file sli_buffer_manager.h
 * @brief
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

#ifndef SLI_BUFFER_MANAGER_H
#define SLI_BUFFER_MANAGER_H

#include "sli_buffer_manager_types.h"
#include "sl_status.h"

/**
 * @brief Initialize the buffer manager.
 * @param configuration Pointer to the configuration.
 */
sl_status_t sli_buffer_manager_init(sli_buffer_manager_configuration_t *configuration);

/**
 * @brief Deinitialize the buffer manager.
 * @return sl_status_t 
 */
sl_status_t sli_buffer_manager_deinit(void);

/**
 * @brief Allocate a buffer.
 * @param pool_type Buffer manager pool Type.
 * @param allocation_type Buffer manager pool type.
 * @param wait_duration_ms Duration to wait for the buffer to be allocated.
 * @param buffer Pointer to the buffer. This pointer points to valid memory in case of successful allocation.
 *               If the allocation fails, it points to null.
 */
sl_status_t sli_buffer_manager_allocate_buffer(const sli_buffer_manager_pool_types_t pool_type,
                                               const sli_buffer_manager_allocation_types_t allocation_type,
                                               const uint32_t wait_duration_ms,
                                               sli_buffer_t *buffer);

/**
 * @brief Free a buffer.
 * @param buffer Pointer to the buffer which needs to be freed.
 */
sl_status_t sli_buffer_manager_free_buffer(sli_buffer_t buffer);
#endif
