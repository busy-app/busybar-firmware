/***************************************************************************//**
 * @file
 * @brief Power Manager execution modes API definition.
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

#include "sl_power_manager_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

#if defined(SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN) && (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN == 1)

/*******************************************************************************
 *****************************   HAL PROTOTYPES   ******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initializes execution mode feature.
 *
 * @note FOR INTERNAL USE ONLY.
 ******************************************************************************/
void sli_power_manager_executions_modes_hal_init(void);

/***************************************************************************//**
 * Applies performance mode execution.
 *
 * @note FOR INTERNAL USE ONLY.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_POWER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_power_manager_hal_apply_performance_mode(void);

/***************************************************************************//**
 * Applies standard mode execution.
 *
 * @note FOR INTERNAL USE ONLY.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_POWER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_power_manager_hal_apply_standard_mode(void);
#endif // (SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN)

#ifdef __cplusplus
}
#endif
