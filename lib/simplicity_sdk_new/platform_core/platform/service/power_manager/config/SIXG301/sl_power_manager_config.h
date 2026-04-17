/***************************************************************************//**
 * @file
 * @brief Power Manager configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_POWER_MANAGER_CONFIG_H
#define SL_POWER_MANAGER_CONFIG_H

// <h>Power Manager Configuration

// <q SL_POWER_MANAGER_CUSTOM_HF_OSCILLATOR_IRQ_HANDLER> Enable custom IRQ handler for external HF oscillator.
// <i> Enable if CMU_IRQHandler/HFXO0_IRQHandler is needed from your application.
// <i> The function sl_power_manager_irq_handler() will have to be called from you custom handler if this is enabled.
// <i> Default: 0
#ifndef SL_POWER_MANAGER_CUSTOM_HF_OSCILLATOR_IRQ_HANDLER
#define SL_POWER_MANAGER_CUSTOM_HF_OSCILLATOR_IRQ_HANDLER  0
#endif

// <q SL_POWER_MANAGER_CONFIG_VOLTAGE_SCALING_FAST_WAKEUP> Enable fast wakeup (disable voltage scaling in EM2/3 mode)
// <i> Enable or disable voltage scaling in EM2/3 modes (when available). This decreases wakeup time by about 30 us.
// <i> Deprecated. It is replaced by the function sl_power_manager_em23_voltage_scaling_enable_fast_wakeup()
// <i> Default: 0
#ifndef SL_POWER_MANAGER_CONFIG_VOLTAGE_SCALING_FAST_WAKEUP
#define SL_POWER_MANAGER_CONFIG_VOLTAGE_SCALING_FAST_WAKEUP   0
#endif

// <e SL_POWER_MANAGER_DEBUG> Enable debugging feature
// <i> Enable or disable debugging features (trace the different modules that have requirements).
// <i> Default: 0
#ifndef SL_POWER_MANAGER_DEBUG
#define SL_POWER_MANAGER_DEBUG  0
#endif

// <o SL_POWER_MANAGER_DEBUG_POOL_SIZE> Maximum numbers of requirements that can be logged
// <i> Default: 10
#ifndef SL_POWER_MANAGER_DEBUG_POOL_SIZE
#define SL_POWER_MANAGER_DEBUG_POOL_SIZE  10
#endif
// </e>

// <o SL_POWER_MANAGER_INIT_EMU_EM4_PIN_RETENTION_MODE> Pin retention mode
// <i>
// <EMU_EM4CTRL_EM4IORETMODE_DISABLE=> No retention
// <EMU_EM4CTRL_EM4IORETMODE_EM4EXIT=> Retention through EM4
// <EMU_EM4CTRL_EM4IORETMODE_SWUNLATCH=> Retention through EM4 and wakeup
// <d> power_manager_pin_retention_disable
#ifndef SL_POWER_MANAGER_INIT_EMU_EM4_PIN_RETENTION_MODE
#define SL_POWER_MANAGER_INIT_EMU_EM4_PIN_RETENTION_MODE EMU_EM4CTRL_EM4IORETMODE_DISABLE
#endif

// <e SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN> Enable Execution Modes feature
// <i> This will override the SL_CLOCK_MANAGER_SYSCLK_SOURCE config in Clock Manager.
// <i> The SYSCLK source will be hardcoded to HFRCODPLL in standard execution mode and will be changed to SOCPLL when switching to performance mode.
// <i> Default: 0
#ifndef SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN
#define SL_POWER_MANAGER_EXECUTION_MODES_FEATURE_EN  0
#endif
// </e>

// <e SL_POWER_MANAGER_SYSCLK_SWITCH_TO_HFXO_IN_SLEEP_EN> Enable SYSCLK on SOCPLL to switch to HFXO for sleep
// <i> Enable/Disable the SYSCLK switching to HFXO when entering sleep.
// <i> By default SYSCLK uses a SOCPLL. To further reduce the power consumption in sleep, the SYSCLK can be switched to HFXO during the sleep period.
// <i> Be aware that the PLL lock time is around 35us, so this will add to the restore time delay when the system wakes up from sleep.
// <i> Default: 1
#ifndef SL_POWER_MANAGER_SYSCLK_SWITCH_TO_HFXO_IN_SLEEP_EN
#define SL_POWER_MANAGER_SYSCLK_SWITCH_TO_HFXO_IN_SLEEP_EN  1
#endif
// </e>
// </h>

#endif /* SL_POWER_MANAGER_CONFIG_H */

// <<< end of configuration section >>>
