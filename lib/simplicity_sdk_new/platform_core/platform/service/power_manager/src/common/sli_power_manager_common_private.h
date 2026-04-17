/***************************************************************************//**
 * @file
 * @brief Power Manager Internal API definition.
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

#include "sl_power_manager.h"
#include "em_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#if (defined(_EMU_STATUS_VSCALE_MASK) || defined(_EMU_CTRL_EM23VSCALE_MASK)) \
  && !defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
// Voltage scaling for EM23 present.
#define SLI_POWER_MANAGER_VSCALE_EM23_PRESENT
#if defined(EMU_CMD_EM01VSCALE1) && !defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
// Voltage scaling for EM01 present.
#define SLI_POWER_MANAGER_VSCALE_EM01_PRESENT
#define SLI_POWER_MANAGER_VSCALE1_MAX_VOLTAGE_CLOCK     40000000UL
#endif
#endif

/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Save the information about the current oscillators used by SYSCLK, and the
 * usage of other HF oscillators.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_POWER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_power_manager_save_oscillators_usage(void);

/*******************************************************************************
 * Gets the delay associated the wake-up process from EM23.
 *
 * @return Delay for the complete wake-up process with full restore.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_POWER_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sli_power_manager_get_wakeup_process_time_overhead(void);

#ifdef __cplusplus
}
#endif
