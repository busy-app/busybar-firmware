/******************************************************************************
* @file  clock_update.h
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

// Includes Files

#include "base_types.h"

#ifndef RSI_CLOCK_UPDATE_H
#define RSI_CLOCK_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PERI_CLKS {
  M4_USART0,  // Enables or Disables USART1 Peripheral clock when it is passed
  M4_UART1,   // Enables or Disables USART2 Master Peripheral clock when it is passed
  M4_SSI_MST, // Enables or Disables SSI Master Peripheral clock when it is passed
  M4_CT,      // Enables or Disables CT Peripheral clock when it is passed
#if !defined(SLI_SI917)
  M4_SD_MEM, // Enables or Disables SD_MEM Peripheral clock when it is passed
  M4_CCI,    // Enables or Disables CCI Peripheral clock when it is passed
#endif
  M4_QSPI, // Enables or Disables QSPI Peripheral clock when it is passed
  M4_QSPI2,
  M4_GSPI,     // Enables or Disables GSPI Peripheral clock when it is passed
  M4_ETHERNET, // Enables or Disables ETHERNET Peripheral clock when it is passed
  M4_I2SM,     // Enables or Disables I2SM Peripheral clock when it is passed
  ULPSS_SSI,
  ULPSS_I2S,
  ULPSS_UART,
  ULPSS_TIMER,
  ULPSS_AUX,
  //ULPSS_VAD,
  ULPSS_TOUCH,
} PERI_CLKS_T;

uint32_t RSI_CLK_GetBaseClock(PERI_CLKS_T peri_src);
uint32_t GetTickCount(void);
void SysTick_Handler(void);

#ifdef __SYSTICK
void rsi_delay_ms(uint32_t val);
#endif

#ifdef __cplusplus
}
#endif

#endif // RSI_CLOCK_UPDATE_H
