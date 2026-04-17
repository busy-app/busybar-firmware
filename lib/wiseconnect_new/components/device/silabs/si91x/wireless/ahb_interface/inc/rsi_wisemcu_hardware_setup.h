/***************************************************************************/ /**
 * @file  rsi_wisemcu_hardware_setup.h
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

#ifndef __RSI_HW_SETUP_H__
#define __RSI_HW_SETUP_H__
/*Includes*/

//
#include "rsi_power_save.h"
#include "base_types.h"

/*Configure the PMU and XTAL good times */
#define PMU_GOOD_TIME_VALUE  0x34 /* 900 us*/
#define XTAL_GOOD_TIME_VALUE 31   /*Duration in us*/

#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
#define NWPAON_MEM_HOST_ACCESS_CTRL_SET_1     (*(volatile uint32_t *)(0x41300000 + 0x0))
#define NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR_1   (*(volatile uint32_t *)(0x41300000 + 0x4))
#define M4SS_TASS_CTRL_SET_REG                (*(volatile uint32_t *)(0x24048400 + 0x34))
#define M4SS_TASS_CTRL_CLR_REG                (*(volatile uint32_t *)(0x24048400 + 0x38))
#define M4SS_REF_CLK_MUX_CTRL                 BIT(24)
#define TASS_REF_CLK_MUX_CTRL                 BIT(25)
#define M4SS_CTRL_TASS_AON_PWR_DMN_RST_BYPASS BIT(2)
#endif

#ifdef CHIP_9118
/*Macro to be used for configuring the 'rams_in_use' parameter in 'sl_si91x_configure_ram_retention()' function*/
#define WISEMCU_0KB_RAM_IN_USE                                                                                        \
  (RAM_BANK_0 | RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_3 | RAM_BANK_4 | RAM_BANK_5 | RAM_BANK_6 | RAM_BANK_7 | RAM_BANK_8 \
   | RAM_BANK_9 | RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_16KB_RAM_IN_USE                                                                            \
  (RAM_BANK_0 | RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_7 | RAM_BANK_8 | RAM_BANK_9 | RAM_BANK_10 | RAM_BANK_11 \
   | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_48KB_RAM_IN_USE                                                                             \
  (RAM_BANK_0 | RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_8 | RAM_BANK_9 | RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 \
   | RAM_BANK_13)
#define WISEMCU_112KB_RAM_IN_USE \
  (RAM_BANK_0 | RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_9 | RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_128KB_RAM_IN_USE \
  (RAM_BANK_0 | RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_144KB_RAM_IN_USE (RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_176KB_RAM_IN_USE (RAM_BANK_2 | RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_192KB_RAM_IN_USE (RAM_BANK_10 | RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_208KB_RAM_IN_USE (RAM_BANK_11 | RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_240KB_RAM_IN_USE (RAM_BANK_12 | RAM_BANK_13)
#define WISEMCU_320KB_RAM_IN_USE (RAM_BANK_13)
#define WISEMCU_384KB_RAM_IN_USE (0)
#endif

#if defined(SLI_SI917)
/*Macro to be used for configuring the 'rams_in_use' parameter in 'sl_si91x_configure_ram_retention()' function*/
#define WISEMCU_0KB_RAM_IN_USE                                                                                        \
  (RAM_BANK_0 | RAM_BANK_1 | RAM_BANK_2 | RAM_BANK_3 | RAM_BANK_4 | RAM_BANK_5 | RAM_BANK_6 | RAM_BANK_7 | RAM_BANK_8 \
   | RAM_BANK_9)
#define WISEMCU_16KB_RAM_IN_USE  (RAM_BANK_4 | RAM_BANK_5 | RAM_BANK_6 | RAM_BANK_7 | RAM_BANK_8 | RAM_BANK_9)
#define WISEMCU_64KB_RAM_IN_USE  (RAM_BANK_6 | RAM_BANK_7 | RAM_BANK_8 | RAM_BANK_9)
#define WISEMCU_128KB_RAM_IN_USE (RAM_BANK_7 | RAM_BANK_8 | RAM_BANK_9)
#define WISEMCU_192KB_RAM_IN_USE (RAM_BANK_8 | RAM_BANK_9)
#define WISEMCU_256KB_RAM_IN_USE (RAM_BANK_9)
#define WISEMCU_320KB_RAM_IN_USE (0U)

/*Macros are used for antenna front switch-end controls */
#define FRONT_END_SWITCH_SEL0 0
#define FRONT_END_SWITCH_SEL1 BIT(29)
#define FRONT_END_SWITCH_SEL2 BIT(30)
#define FRONT_END_SWITCH_SEL3 (BIT(29) | BIT(30))
void RSI_Wireless_GPIO_Frontend_Switch_Controls(void);
#endif
/*Macro used to define the PTE CRC value of the Firmware 17 Boards*/
#define FIRMWARE_17_PTE_CRC_VALUE 0

/*Macro to be used for configuring the 'rams_retention_during_sleep' parameter in 'sl_si91x_configure_ram_retention()' function*/
#define WISEMCU_RETAIN_DEFAULT_RAM_DURING_SLEEP (1)
#define WISEMCU_RETAIN_16K_RAM_DURING_SLEEP     (M4ULP_RAM16K_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_128K_RAM_DURING_SLEEP    (M4ULP_RAM16K_RETENTION_MODE_EN | M4ULP_RAM_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_192K_RAM_DURING_SLEEP \
  (M4ULP_RAM16K_RETENTION_MODE_EN | M4ULP_RAM_RETENTION_MODE_EN | M4SS_RAM_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_384K_RAM_DURING_SLEEP \
  (M4ULP_RAM16K_RETENTION_MODE_EN | M4ULP_RAM_RETENTION_MODE_EN | M4SS_RAM_RETENTION_MODE_EN | TA_RAM_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_M4SS_RAM_DURING_SLEEP  (M4SS_RAM_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_ULPSS_RAM_DURING_SLEEP (ULPSS_RAM_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_TASS_RAM_DURING_SLEEP  (TA_RAM_RETENTION_MODE_EN)
#define WISEMCU_RETAIN_M4ULP_RAM_DURING_SLEEP (M4ULP_RAM_RETENTION_MODE_EN)

/*Function proto-types*/
void sli_si91x_configure_wireless_frontend_controls(uint32_t switch_sel);
void sl_si91x_hardware_setup(void);
void sl_si91x_trigger_sleep(SLEEP_TYPE_T sleepType,
                            uint8_t lf_clk_mode,
                            uint32_t stack_address,
                            uint32_t jump_cb_address,
                            uint32_t vector_offset,
                            uint32_t mode);
void sl_si91x_configure_ram_retention(uint32_t rams_in_use, uint32_t rams_retention_during_sleep);

/*End of file not truncated*/
#endif