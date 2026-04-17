/*******************************************************************************
* @file  sl_opamp_board.h
* @brief sl opamp board specific configuration
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

#ifndef __SL_OPAMP_BOARD_H__
#define __SL_OPAMP_BOARD_H__
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

#ifndef OPAMP1_OUT_LOC
#define SL_OPAMP1_P_PORT RTE_OPAMP1_P_PORT
#define SL_OPAMP1_P_PIN  RTE_OPAMP1_P_PIN
#define SL_OPAMP1_P_MUX  RTE_OPAMP1_P_PORT_MUX

#define SL_OPAMP1_N_PORT RTE_OPAMP1_N_PORT
#define SL_OPAMP1_N_PIN  RTE_OPAMP1_N_PIN
#define SL_OPAMP1_N_MUX  RTE_OPAMP1_N_PORT_MUX

#define SL_OPAMP1_RES_PORT RTE_OPAMP1_RES_PORT
#define SL_OPAMP1_RES_PIN  RTE_OPAMP1_RES_PIN
#define SL_OPAMP1_RES_MUX  RTE_OPAMP1_RES_PORT_MUX

#define SL_OPAMP1_OUT_PORT RTE_OPAMP1_OUT_PORT
#define SL_OPAMP1_OUT_PIN  RTE_OPAMP1_OUT_PIN
#define SL_OPAMP1_OUT_MUX  RTE_OPAMP1_OUT_PORT_MUX
#endif

#ifndef OPAMP2_OUT_LOC
#define SL_OPAMP2_P_PORT RTE_OPAMP2_P_PORT
#define SL_OPAMP2_P_PIN  RTE_OPAMP2_P_PIN
#define SL_OPAMP2_P_MUX  RTE_OPAMP2_P_PORT_MUX

#define SL_OPAMP2_N_PORT RTE_OPAMP2_N_PORT
#define SL_OPAMP2_N_PIN  RTE_OPAMP2_N_PIN
#define SL_OPAMP2_N_MUX  RTE_OPAMP2_N_PORT_MUX

#define SL_OPAMP2_RES_PORT RTE_OPAMP2_RES_PORT
#define SL_OPAMP2_RES_PIN  RTE_OPAMP2_RES_PIN
#define SL_OPAMP2_RES_MUX  RTE_OPAMP2_RES_PORT_MUX

#define SL_OPAMP2_OUT_PORT RTE_OPAMP2_OUT_PORT
#define SL_OPAMP2_OUT_PIN  RTE_OPAMP2_OUT_PIN
#define SL_OPAMP2_OUT_MUX  RTE_OPAMP2_OUT_PORT_MUX
#endif

#ifndef OPAMP3_OUT_LOC
#define SL_OPAMP3_P_PORT RTE_OPAMP3_P_PORT
#define SL_OPAMP3_P_PIN  RTE_OPAMP3_P_PIN
#define SL_OPAMP3_P_MUX  RTE_OPAMP3_P_PORT_MUX

#define SL_OPAMP3_N_PORT RTE_OPAMP3_N_PORT
#define SL_OPAMP3_N_PIN  RTE_OPAMP3_N_PIN
#define SL_OPAMP3_N_MUX  RTE_OPAMP3_N_PORT_MUX

#define SL_OPAMP3_RES_PORT RTE_OPAMP3_RES_PORT
#define SL_OPAMP3_RES_PIN  RTE_OPAMP3_RES_PIN
#define SL_OPAMP3_RES_MUX  RTE_OPAMP3_RES_PORT_MUX

#define SL_OPAMP3_OUT_PORT RTE_OPAMP3_OUT_PORT
#define SL_OPAMP3_OUT_PIN  RTE_OPAMP3_OUT_PIN
#define SL_OPAMP3_OUT_MUX  RTE_OPAMP3_OUT_PORT_MUX
#endif

#ifdef __cplusplus
}
#endif

#endif /*__SL_OPAMP_BOARD_H__*/

/* @} end of Board_Drivers */
/* @} end of Board_Specific_Driver */
