/******************************************************************************
* @file  rsi_bod.h
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

#ifndef __RSI_BOD_H__
#define __RSI_BOD_H__
/**
 * Includes
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "si91x_device.h"
#define BGSAMPLE *(volatile uint32_t *)(0x24048140)

#define BOD_CLOCK_DIVISON_FACTOR (*(volatile uint32_t *)(0x24050000) & 0xF)
#define BOD_ULP_PROC_CLK_SEL     (*(volatile uint32_t *)(24041414) & 0x1E)

#define NPSS_GPIO_2_ANALOG_MODE       *(volatile uint32_t *)0x24048624
#define NPSS_GPIO_2_ANALOG_MODE_VALUE 0x7

#define BUTTON_RANGE_VALUE_CLR 0x3
#define BOD_COMP_SEL_REG_CLR   0x3

#define ENABLE  1
#define DISABLE 0

#define STATUS_EFUSE                      1
#define BOD_COMP_MODE_REG                 0x1E0
#define BOD_COMP_SEL_REG                  0x1E1
#define BOD_BUTTON_REG                    0x1E2
#define BOD_VBATT_STATUS_REG              0x1E3
#define MAX_BOD_THRSHOLD_VALUE            38
#define MAX_MODE_VALUE                    2
#define AUTOMATIC_BOD                     1
#define THRESHOLD_OFFSET_VALUE            0.5
#define THRESHOLD_ROUNDUP_VALUE           1
#define NPSS_TO_MCU_BOD_INTR              BIT(11)
#define BOD_PWRGATE_EN_N_ULP_BUTTON_CALIB BIT(15)
#define PERIODIC_TRIGGER_EN               BIT(8)
#define CHECK_VBATT_STATUS                BIT(7)
#define BATT_SCALE_FACTOR_BITS            0x7E
#define BOD_VBATT_STATUS_REG_BITS         0x3F
#define SLOT_VALUE_BITS                   0x1FFFE

#define MANUAL_CMP_MUX_SEL_BUTTON_CLR   0xE0000
#define MANUAL_CMP_MUX_SEL_BUTTON_VALUE 0xA0000
#define MANUAL_CMP_MUX_SEL_BOD_CLR      0xE0000

#define BUTTON_RANGE_VALUE     0xC1558
#define READ_BUTTON_VALUE_BITS 0x70000
#define BUTTON_ONE_VALUE       4
#define BUTTON_THREE_VALUE     1
#define BUTTON_TWO_VALUE       2

#define BUTTON_ONE   1
#define BUTTON_TWO   2
#define BUTTON_THREE 3

#define BOD_TEST_SEL_BITS_CLR 0x180000

#define MANUAL_CMP_MUX_SEL_VALUE 0x80000
#define ENABLE_IN_SLEEP_MODE     0x03
#define DISABLE_IN_SLEEP_MODE    0x0

#define BATTERY_STATUS_MAX              38
#define BOD_VBATT_COMPUT_FIRST_ELEMENT  3.3f
#define BOD_VBATT_COMPUT_SECOND_ELEMENT 34.814f
#define BOD_VBATT_COMPUT_THIRD_ELEMENT  31

#define SLOT_MAX_VALUE 32767
#define SLOT_MIN_VALUE 0

#define BOD_CLEAR_BATTERY_STATUS_BITS_EFUSE 0x3FFFFF

float RSI_BOD_SoftTriggerGetBatteryStatus(void);
float RSI_BOD_PeriodicTriggerGetBatteryStatus(void);
void RSI_BOD_Enable(uint8_t enable, float vbatt_threshold);
uint32_t RSI_BOD_GetThreshold(void);
rsi_error_t RSI_BOD_ConfigSlotValue(uint16_t slot_value);
void RSI_BOD_ButtonWakeUpEnable(uint8_t enable);
uint32_t RSI_BOD_Buttonvalue(void);
rsi_error_t RSI_BOD_CMP_Hysteresis(uint8_t value);
rsi_error_t RSI_BOD_SetMode(uint8_t mode);
void RSI_BOD_BlackOutReset(uint16_t enable);

void RSI_BOD_BGSampleEnable(void);
void RSI_BOD_BGSampleDisable(void);
rsi_error_t RSI_BOD_BodTestSel(uint8_t enable, uint8_t bod_test_sel_value);

void RSI_BOD_IntrEnable(uint16_t enable);
void RSI_BOD_ButtonIntrEnable(uint16_t enable);
void RSI_BOD_IntrClr(void);
void RSI_BOD_ButtonIntrClr(void);
uint32_t RSI_BOD_GetIntrStatus(void);
void IRQ023_Handler(void);

#ifdef __cplusplus
}

#endif

#endif /*__RSI_BOD_H__ */
