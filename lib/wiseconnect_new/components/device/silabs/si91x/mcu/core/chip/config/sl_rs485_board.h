/*******************************************************************************
* @file  sl_rs485_board.h
* @brief sl rs485 board specific configuration
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_RS485_BOARD_H__
#define __SL_RS485_BOARD_H__
/**
 * @defgroup  Board_Specific_Driver   Board specific drivers and support functions
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup  Board_Drivers BOARD: Common board components used with board drivers
 * @{
 */
#include "em_device.h"

#define SL_RS485_UART0_RE_PIN  RTE_UART0_RS485_RE_PIN
#define SL_RS485_UART0_RE_PORT RTE_UART0_RS485_RE_PORT
#define SL_RS485_UART0_RE_MUX  RTE_UART0_RS485_RE_MUX
#define SL_RS485_UART0_RE_PAD  RTE_UART0_RS485_RE_PAD

#define SL_RS485_UART0_DE_PIN  RTE_UART0_RS485_DE_PIN
#define SL_RS485_UART0_DE_PORT RTE_UART0_RS485_DE_PORT
#define SL_RS485_UART0_DE_MUX  RTE_UART0_RS485_DE_MUX
#define SL_RS485_UART0_DE_PAD  RTE_UART0_RS485_DE_PAD

#define SL_RS485_UART0_EN_PIN  RTE_UART0_RS485_EN_PIN
#define SL_RS485_UART0_EN_PORT RTE_UART0_RS485_EN_PORT
#define SL_RS485_UART0_EN_MUX  RTE_UART0_RS485_EN_MUX
#define SL_RS485_UART0_EN_PAD  RTE_UART0_RS485_EN_PAD

#define SL_RS485_UART1_RE_PIN  RTE_UART1_RS485_RE_PIN
#define SL_RS485_UART1_RE_PORT RTE_UART1_RS485_RE_PORT
#define SL_RS485_UART1_RE_MUX  RTE_UART1_RS485_RE_MUX
#define SL_RS485_UART1_RE_PAD  RTE_UART1_RS485_RE_PAD

#define SL_RS485_UART1_DE_PIN  RTE_UART1_RS485_DE_PIN
#define SL_RS485_UART1_DE_PORT RTE_UART1_RS485_DE_PORT
#define SL_RS485_UART1_DE_MUX  RTE_UART1_RS485_DE_MUX
#define SL_RS485_UART1_DE_PAD  RTE_UART1_RS485_DE_PAD

#define SL_RS485_UART1_EN_PIN  RTE_UART1_RS485_EN_PIN
#define SL_RS485_UART1_EN_PORT RTE_UART1_RS485_EN_PORT
#define SL_RS485_UART1_EN_MUX  RTE_UART1_RS485_EN_MUX
#define SL_RS485_UART1_EN_PAD  RTE_UART1_RS485_EN_PAD

#ifdef __cplusplus
}
#endif

#endif /*__SL_RS485_BOARD_H__*/

/* @} end of Board_Drivers */
/* @} end of Board_Specific_Driver */
