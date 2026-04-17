/*******************************************************************************
* @file  rsi_debug.h
* @brief 
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

#ifndef __RSI_DEBUG_H__
#define __RSI_DEBUG_H__

#include <stdio.h>
#include <stdint.h>
#include "rsi_ccp_user_config.h"
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif /* SL_COMPONENT_CATALOG_PRESENT */
#ifdef DEBUG_UART_UC
#include "sl_si91x_debug_uc_config.h"
#endif
#ifdef SL_CATALOG_SI91X_IOSTREAM_PRINTS_PRESENT
#include "sl_iostream.h"
#include "sl_si91x_iostream_log.h"
#include "sl_si91x_iostream_log_config.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

/*Sample print prototype*/
void Board_Debug_Init(void);
void Board_UARTPutSTR(const uint8_t *ptr);
uint8_t Board_UARTGetChar(void);
void Board_UARTPutChar(uint8_t ch);
void dummy_printf(const char *fmt, ...);

// Define debug macros based on configuration
#if defined(IOSTREAM_USART) || SL_SI91X_IOSTREAM_LOG_PRINTS_ENABLE
// SI91X IOStream prints configuration
#if SL_SI91X_IOSTREAM_LOG_PRINTS_ENABLE
#define DEBUGINIT() sl_si91x_iostream_log_init()
#else
#define DEBUGINIT()
#endif
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) (void)str
#define DEBUGIN()

#elif defined(DEBUG_UART)
// Debug over UART configuration
#define DEBUGINIT()   Board_Debug_Init()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) Board_UARTPutSTR(str)
#define DEBUGIN()     Board_UARTGetChar()

#else
// No debug output configuration
#define DEBUGINIT()
#define DEBUGOUT(...) dummy_printf(__VA_ARGS__)
#define DEBUGSTR(str) (void)str
#define DEBUGIN()
#endif

#ifdef __cplusplus
}
#endif

#endif // __RSI_DEBUG_H__
