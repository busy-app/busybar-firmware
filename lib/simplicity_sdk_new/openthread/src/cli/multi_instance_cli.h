/*******************************************************************************
 * @file
 * @brief Multi-instance CLI support header
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

#ifndef MULTI_INSTANCE_CLI_H
#define MULTI_INSTANCE_CLI_H

#include <stdbool.h>
#include <openthread/cli.h>
#include <openthread/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

#if SL_OPENTHREAD_MULTI_INSTANCE_CLI_ENABLE

/**
 * Get the current instance index.
 *
 * @returns The current instance index.
 */
uint8_t sl_ot_get_current_instance_index(void);

/**
 * Set the new instance index to switch to.
 *
 * @param[in] aInstanceIndex The instance index to switch to.
 */
void sl_ot_set_new_instance_index(uint8_t aInstanceIndex);

/**
 * Get the new instance index that was set for switching.
 *
 * @returns The new instance index.
 */
uint8_t sl_ot_get_new_instance_index(void);

/**
 * Set the current instance index.
 *
 * @param[in] aInstanceIndex The instance index to set as current.
 */
void sl_ot_set_current_instance_index(uint8_t aInstanceIndex);

/**
 * Switch to the specified instance index.
 *
 * @param[in] aInstanceIndex The instance index to switch to.
 */
void sl_ot_switch_to_instance_index(uint8_t aInstanceIndex);

/**
 * Check if the instance should be changed.
 *
 * @returns True if the instance should be changed, false otherwise.
 */
bool sl_ot_should_change_instance(void);

#endif // SL_OPENTHREAD_MULTI_INSTANCE_CLI_ENABLE

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MULTI_INSTANCE_CLI_H
