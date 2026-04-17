/******************************************************************************
* @file sl_si91x_m4_ps.h
* @brief  M4 power save
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

#ifndef SL_SI91X_M4_POWERSAVE_H_
#define SL_SI91X_M4_POWERSAVE_H_

#ifdef DEBUG_UART
#include "rsi_debug.h"
#endif
#ifdef SLI_SI91X_MCU_INTERFACE
#include "rsi_rtc.h"
#include "rsi_m4.h"
#include "rsi_ds_timer.h"
#include "rsi_wisemcu_hardware_setup.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_rom_timer.h"
#include "rsi_rom_power_save.h"
#ifndef _ZEPHYR_
#include "sl_event_handler.h"
#endif
/*=======================================================================*/
//! Power save command parameters
/*=======================================================================*/
//! set handshake type of power mode
#define RSI_HAND_SHAKE_TYPE     M4_BASED
#define WIRELESS_WAKEUP_IRQ_PRI 8

// -----------------------------------------------------------------------------
// Prototypes
/**
 * @fn           initialize_m4_alarm.
 * @brief        This function is to initialize Alarm block .
 * @param[in]  none
 * @return    none.
 */
void initialize_m4_alarm(void);
void IRQ026_Handler();
void vPortSetupTimerInterrupt(void);
void sli_si91x_m4_ta_wakeup_configurations(void);
void wakeup_source_config(void);
#endif /* SLI_SI91X_MCU_INTERFACE */
#endif /* SL_SI91X_M4_POWERSAVE_H_ */