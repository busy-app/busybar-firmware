/*******************************************************************************
* @file  rsi_debug.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

#ifndef __RSI_DEBUG_H__
#define __RSI_DEBUG_H__

#include <stdio.h>
#include <stdint.h>
#include <furi/core/log.h>
//#include "rsi_ccp_user_config.h"
#ifdef DEBUG_UART_UC
#ifndef SL_SI91X_ULP_UART
#include "sl_si91x_debug_uc_1_config.h"
#else
#include "sl_si91x_debug_uc_2_config.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*Sample print prototype*/
// void Board_Debug_Init(void);

#include <core/common_defines.h>

// void Board_UARTPutSTR(const uint8_t *ptr);
// uint8_t Board_UARTGetChar(void);
// void Board_UARTPutChar(uint8_t ch);
// void dummy_printf(const char *fmt, ...);

static FURI_ALWAYS_INLINE void Board_UARTPutSTR(const uint8_t* ptr) {
    furi_log_puts((const char*)ptr);
}

#ifndef IOSTREAM_USART
#ifdef DEBUG_UART
#define DEBUGINIT()   Board_Debug_Init()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) Board_UARTPutSTR(str)
#define DEBUGIN()     Board_UARTGetChar()
#else
#define DEBUGINIT()
#define DEBUGOUT(...) dummy_printf(__VA_ARGS__)
#define DEBUGSTR(str) (void)str
#define DEBUGIN()
#endif
#else
#define DEBUGINIT()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif // __RSI_DEBUG_H__
